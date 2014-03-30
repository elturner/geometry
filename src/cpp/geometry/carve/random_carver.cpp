#include "random_carver.h"
#include <mesh/floorplan/floorplan.h>
#include <geometry/shapes/extruded_poly.h>
#include <geometry/shapes/chunk_exporter.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/frame_model.h>
#include <geometry/octree/octree.h>
#include <timestamp/sync_xml.h>
#include <io/data/fss/fss_io.h>
#include <io/carve/chunk_io.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <boost/threadpool.hpp>
#include <boost/thread.hpp>
#include <Eigen/Dense>
#include <limits.h>
#include <iostream>
#include <string>
#include <map>
#include <set>

/**
 * @file random_carver.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the random_carver_t class, which
 * is used to populate an octree with imported range scans,
 * in a probalistic fashion.
 *
 * This file requires the Eigen framework to be linked.
 */

using namespace std;

/* function implementations */
		
random_carver_t::random_carver_t()
{
	/* default parameter values */
	this->num_rooms = 0;
	this->num_threads = 1;
}

int random_carver_t::init(const string& madfile, const string& confile,
                          const string& tsfile,
                          double res, double dcu, double carvebuf,
			  unsigned int nt)
{
	int ret;

	/* initialize the structures */
	ret = this->path.readmad(madfile);
	if(ret)
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to initialize"
		     << " path of system, Error " << ret << endl;
		return PROPEGATE_ERROR(-1, ret); /* cannot read file */
	}
	ret = this->path.parse_hardware_config(confile);
	if(ret)
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to initialize"
		     << " system hardware config, Error " << ret << endl;
		return PROPEGATE_ERROR(-2, ret); /* cannot read file */
	}
	if(!this->timesync.read(tsfile))
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to parse the "
		     << "time synchronization output xml file: "
		     << tsfile << endl;
		return PROPEGATE_ERROR(-3, ret);
	}

	/* initialize octree and algorithm parameters */
	this->tree.set_resolution(res);
	this->carving_buffer = carvebuf;
	this->default_clock_uncertainty = dcu;
	this->num_rooms = 0;
	this->num_threads = nt;

	/* success */
	return 0;
}
		
int random_carver_t::export_chunks(const vector<string>& fss_files,
                                   const string& chunklist,
                                   const string& chunk_dir)
{
	chunk_exporter_t chunker;
	vector<string> sensor_names;
	fss::reader_t infile;
	fss::frame_t inframe;
	progress_bar_t progbar;
	scan_model_t model;
	frame_model_t curr_frame, prev_frame;
	string label;
	tictoc_t clk;
	unsigned int i, si, n, num_sensors;
	int ret;

	/* open chunk exporter */
	chunker.open(chunklist, chunk_dir);

	/* iterate over scan files */
	num_sensors = fss_files.size();
	sensor_names.resize(num_sensors);
	for(si = 0; si < num_sensors; si++)
	{
		/* read in scan file */
		tic(clk);
		infile.set_correct_for_bias(true); /* correct bias */
		infile.set_convert_to_meters(true); /* units: meters */
		ret = infile.open(fss_files[si]);
		if(ret)
		{
			/* unable to parse, report to user */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Error " << ret
			     << ": Unable to parse input scan file "
			     << fss_files[si] << endl;
			return ret;
		}

		/* record name of sensor */
		sensor_names[si] = infile.scanner_name();

		/* prepare noisy model for this scanner */
		ret = model.set_sensor(infile.scanner_name(), 
	        	this->get_clock_uncertainty_for_sensor(
				infile.scanner_name()), this->path);
		if(ret)
		{
			/* not a recognized sensor */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Error " << ret
			     << ": Unable to recognize sensor \""
			     << infile.scanner_name() << "\"" << endl;
			return ret;
		}
		label = "Parsing " + infile.scanner_name();
		toc(clk, label.c_str());

		/* iterate through scans */
		tic(clk);
		progbar.set_name(infile.scanner_name());
		n = infile.num_frames();
		for(i = 0; i < n; i++)
		{
			/* inform user of progress */
			progbar.update(i, n);
			
			/* parse current frame, update user on progress */
			ret = infile.get(inframe, i);
			if(ret)
			{
				/* error occurred reading frame, 
				 * inform user */
				progbar.clear();
				ret = PROPEGATE_ERROR(-3, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to parse frame #"
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}
		
			/* compute carving model for this frame */
			ret = curr_frame.init(inframe, model, this->path);
			if(ret)
			{
				/* error occurred computing 
				 * frame parameters */
				progbar.clear();
				ret = PROPEGATE_ERROR(-4, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to compute frame #" 
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}
		
			/* only proceed from here if we have two frame's 
			 * worth of data, so we can interpolate the 
			 * distributions between them and carve the 
			 * corresponding volume. */
			if(i == 0)
			{
				/* prepare for the next frame */
				curr_frame.swap(prev_frame);
				continue;
			}

			/* add frame information to octree */
			ret = prev_frame.export_chunks(
				this->tree, curr_frame,
				this->carving_buffer, si, i-1,
				chunker);
			if(ret)
			{
				/* error occurred carving this frame */
				progbar.clear();
				ret = PROPEGATE_ERROR(-5, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to chunk frame #" << i
				     << ", Error " << ret << endl;
				return ret;
			}

			/* prepare for the next frame */
			curr_frame.swap(prev_frame);
		}

		/* inform user that processing is finished */
		progbar.clear();
		infile.close();
		label = "Chunking of " + infile.scanner_name();
		toc(clk, label.c_str());
	}

	/* close this chunker, which will finish exporting all files */
	chunker.close(this->tree, sensor_names);

	/* success */
	return 0;
}
		
int random_carver_t::carve_all_chunks(
			const vector<string>& fss_files,
			const string& chunklist)
{
	Eigen::Vector3d treecenter;
	vector<string> sensor_names;
	vector<fss::reader_t*> fss_streams;
	vector<double> ts_uncertainties;
	chunk::chunklist_reader_t chunk_infile;
	progress_bar_t progbar;
	unsigned int i, j, num_sensors_in_file, num_sensors_given;
	size_t num_chunks;
	string chunkfile;
	fss::reader_t* swap;
	tictoc_t clk;
	int ret;

	/* initialize threadpool */
	boost::threadpool::pool tp(this->num_threads);

	/* attempt to parse the chunklist file */
	tic(clk);
	ret = chunk_infile.open(chunklist);
	if(ret)
	{
		/* unable to parse */
		cerr << "[random_carver_t::carve_all_chunks]\t"
		     << "Unable to read chunklist file: " << chunklist
		     << endl;
		return PROPEGATE_ERROR(-1, ret);
	}
	
	/* prepare tree for importing chunks.  This will
	 * clear any existing data from tree. */
	treecenter << chunk_infile.center_x(),
	              chunk_infile.center_y(),
	              chunk_infile.center_z();
	this->tree.set(treecenter, chunk_infile.halfwidth(),
	               this->tree.get_resolution());

	/* get sensor ordering */
	chunk_infile.get_sensor_names(sensor_names);
	num_sensors_in_file = sensor_names.size();

	/* open sensor files */
	num_sensors_given = fss_files.size();
	fss_streams.resize(num_sensors_given);
	for(i = 0; i < num_sensors_given; i++)
	{
		/* open this fss file for reading */
		fss_streams[i] = new fss::reader_t();
		fss_streams[i]->set_correct_for_bias(true); 
				/* correct statistical bias */
		fss_streams[i]->set_convert_to_meters(true);
				/* we want units of meters */
		ret = fss_streams[i]->open(fss_files[i]);
		if(ret)
		{
			/* could not open given file */
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "Unable to open fss file: "
			     << fss_files[i] << endl << endl;
			
			/* clean up and return */
			chunk_infile.close();
			for(j = 0; j <= i; j++)
			{
				fss_streams[j]->close();
				delete (fss_streams[j]);
			}
			fss_streams.clear();
			return PROPEGATE_ERROR(-2, ret);
		}
	}

	/* reorder sensors to what chunklist uses */
	for(i = 0; i < num_sensors_in_file; i++)
	{
		/* check all streams for the i'th sensor from file */
		for(j = i; j < num_sensors_given; j++)
		{
			/* is this the right sensor? */
			if(!sensor_names[i].compare(
					fss_streams[j]->scanner_name()))
			{
				/* swap this stream to the desired
				 * position in list */
				swap = fss_streams[j];
				fss_streams[j] = fss_streams[i];
				fss_streams[i] = swap;

				/* done searching */
				break;
			}
		}

		/* check if successful */
		if(j >= num_sensors_given)
		{
			/* could not find necessary sensor */
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "ERROR: A necessary .fss file is missing!"
			     << " Please make sure you include the file "
			     << " for sensor: \"" << sensor_names[i] << "\""
			     << endl << endl;
			
			/* clean up and return */
			chunk_infile.close();
			for(j = 0; j < num_sensors_given; j++)
			{
				fss_streams[j]->close();
				delete (fss_streams[j]);
			}
			fss_streams.clear();
			return PROPEGATE_ERROR(-3, ret);
		}
	}

	/* record the timestamp uncertainties for each sensor */
	ts_uncertainties.resize(num_sensors_given);
	for(j = 0; j < num_sensors_given; j++)
		ts_uncertainties[j] 
			= this->get_clock_uncertainty_for_sensor(
				fss_streams[j]->scanner_name());

	toc(clk, "Parsing chunk list");

	/* iterate over the chunks in this file */
	tic(clk);
	progbar.set_name("Processing chunks");
	num_chunks = chunk_infile.num_chunks();
	for(i = 0; i < num_chunks; i++)
	{
		/* update user */
		progbar.update(i, num_chunks);

		/* get the file for the next chunk */
		ret = chunk_infile.next(chunkfile);
		if(ret)
		{
			/* report error to user and continue */
			ret = PROPEGATE_ERROR(-4, ret);
			progbar.clear();
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "Unable to get uuid for chunk #" << i
			     << ", Error " << ret << endl << endl;
			continue;
		}

		/* process this chunk */
		ret = this->carve_chunk(fss_streams, ts_uncertainties, 
					chunkfile, tp); 
		if(ret)
		{
			/* report error and continue */
			ret = PROPEGATE_ERROR(-5, ret);
			progbar.clear();
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "Error " << ret << ": Unable to process "
			     << "chunk #" << i << ": " << chunkfile
			     << endl << endl;
			continue;
		}

		/* make sure thread pool isn't overfull, just store
		 * enough to be actively processing on all threads,
		 * and have enough scheduled in the queue to refill
		 * those threads. */
		tp.wait(this->num_threads + this->num_threads);
	}

	/* wait for all threads to finish */
	tp.wait();
	
	/* clean up */
	chunk_infile.close();
	for(j = 0; j < num_sensors_given; j++)
	{
		fss_streams[j]->close();
		delete (fss_streams[j]);
	}
	fss_streams.clear();
	progbar.clear();
	toc(clk, "Processing all chunks");

	/* success */
	return 0;
}

int random_carver_t::carve_chunk(
			const vector<fss::reader_t*>& fss_files,
			const vector<double>& ts_uncerts,
			const string& chunkfile,
			boost::threadpool::pool& tp)
{
	Eigen::Vector3d chunkcenter;
	set<chunk::point_index_t> inds;
	chunk::chunk_reader_t infile;
	chunk::point_index_t pi;
	octnode_t* chunknode;
	unsigned int i, n, chunkdepth;
	int ret;

	/* open this chunk file for reading */
	ret = infile.open(chunkfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* import the indices in this chunk */
	n = infile.num_points();
	for(i = 0; i < n; i++)
	{
		/* get the point index structure from file */
		ret = infile.next(pi);
		if(ret)
		{
			/* could not read from file */
			infile.close();
			return PROPEGATE_ERROR(-2, ret);
		}

		/* import this index into index set.  Since the
		 * set is sorted, the indices will be ordered
		 * with sensor-major, then frame, then point-minor
		 * ordering, which allows us to carve everything
		 * efficiently while only preparing each frame
		 * once. */
		inds.insert(pi);
	}

	/* check if file had duplicate indices */
	if(inds.size() != n)
		cerr << "[random_carver_t::carve_chunk]\tInfile had "
		     << n << " points but only " << inds.size()
		     << " were unique." << endl << endl;

	/* prepare chunk location within tree */
	chunkcenter << infile.center_x(),
	               infile.center_y(),
	               infile.center_z();
	infile.close();
	chunknode = this->tree.expand(chunkcenter, infile.halfwidth(),
	                              chunkdepth);
	if(chunknode == NULL)
		return PROPEGATE_ERROR(-3, ret); /* tree not initialized */

	/* process this node based on the input chunk data */
	if(this->num_threads > 1)
	{
		/* the input settings say to multithread this
		 * processing, so schedule the carving of this node
		 * into the threadpool */
		tp.schedule(boost::bind(random_carver_t::carve_node,
			chunknode, inds, boost::cref(fss_files),
			boost::cref(ts_uncerts), boost::cref(path),
			chunkdepth, this->carving_buffer));
	}
	else
	{
		/* since we're not going to use multiple threads, don't
		 * bother using the threadpool at all, and instead just
		 * process the chunk using a direct function call */
		random_carver_t::carve_node(chunknode, inds, fss_files,
			ts_uncerts, path, chunkdepth, this->carving_buffer);
	}

	/* clean up */
	return 0;
}

int random_carver_t::carve_direct(const string& fssfile)
{
	fss::reader_t infile;
	fss::frame_t inframe;
	progress_bar_t progbar;
	scan_model_t model;
	frame_model_t curr_frame, prev_frame;
	string label;
	tictoc_t clk;
	unsigned int i, n;
	int ret;

	/* read in scan file */
	tic(clk);
	infile.set_correct_for_bias(true); /* correct statistical bias */
	infile.set_convert_to_meters(true); /* we want units of meters */
	ret = infile.open(fssfile);
	if(ret)
	{
		/* unable to parse, report to user */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::carve]\tError " << ret
		     << ": Unable to parse input scan file "
		     << fssfile << endl;
		return ret;
	}

	/* prepare noisy model for this scanner */
	ret = model.set_sensor(infile.scanner_name(), 
	                       this->get_clock_uncertainty_for_sensor(
	                       infile.scanner_name()), this->path);
	if(ret)
	{
		/* not a recognized sensor */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[random_carver_t::carve]\tError " << ret
		     << ": Unable to recognize sensor \""
		     << infile.scanner_name() << "\"" << endl;
		return ret;
	}
	toc(clk, "Parsing scan file");

	/* iterate through scans, incorporating them into the octree */
	tic(clk);
	progbar.set_name(infile.scanner_name());
	n = infile.num_frames();
	for(i = 0; i < n; i++)
	{
		/* inform user of progress */
		progbar.update(i, n);
		
		/* parse the current frame and update user on progress */
		ret = infile.get(inframe, i);
		if(ret)
		{
			/* error occurred reading frame, inform user */
			progbar.clear();
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "parse frame #" << i << ", Error "
			     << ret << endl;
			return ret;
		}
		
		/* compute carving model for this frame */
		ret = curr_frame.init(inframe, model, this->path);
		if(ret)
		{
			/* error occurred computing frame parameters */
			progbar.clear();
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "compute frame #" << i << ", Error "
			     << ret << endl;
			return ret;
		}
		
		/* only proceed from here if we have two frame's worth
		 * of data, so we can interpolate the distributions
		 * between them and carve the corresponding volume. */
		if(i == 0)
		{
			/* prepare for the next frame */
			curr_frame.swap(prev_frame);
			continue;
		}

		/* add frame information to octree */
		ret = prev_frame.carve(this->tree, curr_frame,
		                       this->carving_buffer);
		if(ret)
		{
			/* error occurred carving this frame */
			progbar.clear();
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "carve frame #" << i << " into tree, "
			     << "Error " << ret << endl;
			return ret;
		}

		/* prepare for the next frame */
		curr_frame.swap(prev_frame);
	}

	/* inform user that processing is finished */
	progbar.clear();
	label = "Random carving of " + infile.scanner_name();
	toc(clk, label.c_str());

	/* success */
	return 0;
}
		
int random_carver_t::import_fp(const std::string& fpfile)
{
	fp::floorplan_t f;
	extruded_poly_t poly;
	tictoc_t clk;
	unsigned int i, n;
	int ret;

	/* read in floor plan */
	tic(clk);
	ret = f.import_from_fp(fpfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* iterate over the rooms of this floorplan, and generate
	 * a shape object for each room */
	n = f.rooms.size();
	for(i = 0; i < n; i++)
	{
		/* create shape */
		poly.init(f, this->num_rooms + i, i);
	
		/* import into tree */
		this->tree.find(poly);
	}

	/* update number of rooms in building */
	this->num_rooms += n;
	toc(clk, "Importing floor plan");

	/* success */
	return 0;
}

int random_carver_t::serialize(const string& octfile) const
{
	tictoc_t clk;
	int ret;

	/* serialize the octree */
	tic(clk);
	ret = this->tree.serialize(octfile);
	if(ret)
	{
		/* unable to write to file, inform user */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::serialize]\tError " << ret
		     << ": Unable to write to output file "
		     << octfile << endl;
		return -1;
	}
	toc(clk, "Exporting octfile");

	/* success */
	return 0;
}
		
double random_carver_t::get_clock_uncertainty_for_sensor(
                        const string& sensor_name) const
{
	FitParams clk;

	/* get the timesync parameters for this sensor */
	clk = this->timesync.get(sensor_name);

	/* check if std. dev. is valid */
	if(clk.stddev < 0)
		return this->default_clock_uncertainty;

	/* return this sensor's specific uncertainty */
	return clk.stddev;
}
		
void random_carver_t::carve_node(octnode_t* chunknode,
			set<chunk::point_index_t> inds,
			const vector<fss::reader_t*>& fss_files,
			const vector<double>& ts_uncerts,
			const system_path_t& path,
			unsigned int maxdepth, double carvebuf)
{
	frame_model_t curr_frame, next_frame;
	set<chunk::point_index_t>::iterator it;
	scan_model_t model;
	fss::frame_t inframe;
	unsigned int si, fi;
	int ret;

	/* iterate over the indices referenced in this chunk */
	si = fi = UINT_MAX; /* set sensor and frame indices to invalid */
	for(it = inds.begin(); it != inds.end(); it++)
	{
		/* check if we need to update sensor */
		if(it->sensor_index != si)
		{
			// TODO only use one sensor
			if(fss_files[it->sensor_index]->scanner_name().compare("H1214157"))
				continue;

			/* update index */
			si = it->sensor_index;

			/* update noise model for this sensor */
			ret = model.set_sensor(
				fss_files[si]->scanner_name(), 
				ts_uncerts[si], path);
			if(ret)
			{
				/* not a recognized sensor */
				cerr << "[carve_node]\tError " << ret << ":"
				     << " Unrecognized sensor: "
				     << fss_files[si]->scanner_name()
				     << endl << endl;
				return;
			}
			
			/* if we updated the sensor, then we 
			 * need to update the frame as well */
			fi = UINT_MAX;
		}

		/* check if need to update frame referenced */
		if(it->frame_index != fi)
		{
			/* check if we are going to the next
			 * adjacent frame, in which case we can
			 * swap current with next */
			if(fi != UINT_MAX && it->frame_index == fi+1
					&& next_frame.get_num_points() > 0)
			{
				/* swap with next to keep using
				 * this frame, since the next frame
				 * is already populated, and what we
				 * want for this frame anyway. */
				curr_frame.swap(next_frame);
			}
			else
			{
				/* need to read in current frame from
				 * scratch */
				ret = fss_files[si]->get(inframe,
						it->frame_index);
				if(ret)
				{
					/* error occurred, abort! */
					cerr << "[carve_node]\tError "
					     << ret << ": Could not get "
					     << "frame " << it->frame_index
					     << " from sensor #" << si
					     << endl << endl;
					return;
				}
				
				/* update model for current frame */
				ret = curr_frame.init(inframe, model,
				                      path);
				if(ret)
				{
					/* error occurred! */
					cerr << "[carve_node]\tError "
					     << ret << ": Could not init "
					     << "frame " << it->frame_index
					     << " of sensor #" << si
					     << endl << endl;
					return;
				}
			}
			
			/* update index */
			fi = it->frame_index;

			/* get next frame, which must always be read
			 * in from scratch */
			ret = fss_files[si]->get(inframe, fi+1);
			if(ret)
			{
				/* error occurred, which may likely be
				 * caused by index overflow if
				 * fi >= num frames.  But if that's
				 * the case, then we want to still return
				 * an error, since the chunk files shouldn't
				 * have that. */
				cerr << "[carve_node]\tError " << ret
				     << ": Could not get nframe " << (fi+1)
				     << " of sensor #" << si << endl
				     << endl;
				return;
			}

			/* update model for next frame */
			ret = next_frame.init(inframe, model, path);
			if(ret)
			{
				/* error occurred in statistical
				 * calculations */
				cerr << "[carve_node]\tError " << ret 
				     << ": Could not init nframe "
				     << (fi+1) << " of sensor #" << si
				     << endl << endl;
				return;
			}
		}

		// TODO planarity/edge info about scan?

		/* carve the referenced wedge */
		ret = curr_frame.carve_single(chunknode, maxdepth,
			next_frame, carvebuf, it->point_index);
		if(ret)
		{
			/* error occurred during carving */
			cerr << "[carve_node]\tError " << ret << ": "
			     << "Could not carve wedge #" << it->point_index
			     << " when current frame is #" << fi
			     << " and current sensor is #" << si
			     << endl << endl;
			return;
		}
	}

	/* simplify this chunk now that it is fully carved */
	chunknode->simplify_recur();
}
