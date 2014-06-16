#include "random_carver.h"
#include <geometry/shapes/chunk_exporter.h>
#include <geometry/shapes/carve_wedge.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/frame_model.h>
#include <geometry/octree/octree.h>
#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
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
	this->num_threads = 1;
}

void random_carver_t::init(double res, unsigned int nt)
{
	/* initialize octree and algorithm parameters */
	this->tree.set_resolution(res);
	this->num_threads = nt;
}
		
int random_carver_t::export_chunks(const string& cmfile,
                                   const string& wedgefile,
                                   const string& chunklist,
                                   const string& chunk_dir)
{
	vector<chunk::point_index_t> vals;
	chunk_exporter_t chunker; /* output */
	cm_io::reader_t cm_infile; /* input */
	wedge::reader_t wedge_infile; /* input */
	unsigned int ia, ib; /* frame indices */
	unsigned int ia1, ia2, ib1, ib2; /* indices of imported carvemaps */
	carve_map_t a1, a2, b1, b2; /* imported carve maps */
	carve_wedge_t w; /* imported wedge, references carve maps */
	progress_bar_t progbar;
	tictoc_t clk;
	size_t i, num_wedges;
	int ret;

	/* open chunk exporter */
	chunker.open(chunklist, chunk_dir);

	/* open carve map file */
	ret = cm_infile.open(cmfile);
	if(ret)
	{
		/* an error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::export_chunks]\tError "
		     << ret << ": Unable to open carve map file: "
		     << cmfile << endl << endl;
		chunker.close(this->tree);
		return ret;
	}

	/* open wedge reader */
	ret = wedge_infile.open(wedgefile);
	if(ret)
	{
		/* an error occurred */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[random_carver_t::export_chunks]\tError "
		     << ret << ": Unable to open wedge file: "
		     << wedgefile << endl;
		chunker.close(this->tree);
		cm_infile.close();
		return ret;
	}

	/* iterate over input wedges */
	tic(clk);
	progbar.set_name("chunking wedges");
	num_wedges = wedge_infile.num_wedges();
	for(i = 0; i < num_wedges; i++)
	{
		/* inform user of progress */
		progbar.update(i, num_wedges);
		
		/* parse current wedge */
		ret = wedge_infile.get(ia, ia1, ia2, ib, ib1, ib2, i);
		if(ret)
		{
			/* error occurred reading wedge,  inform user */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Unable to parse wedge #"
			     << i << ", Error " << ret << endl;
			return ret;
		}

		/* prepare the carve maps to be used for this wedge */
		ret = cm_infile.read(a1, ia, ia1);
		if(ret)
		{
			/* error occurred */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[random_carver_t::export_chunks]\tError "
			     << ret << ": Could not parse carvemap #("
			     << ia << ", " << ia1 << ") for "
			     << "wedge #" << i << endl << endl;
			return ret;
		}
		ret = cm_infile.read(a2, ia, ia2);
		if(ret)
		{
			/* error occurred */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[random_carver_t::export_chunks]\tError "
			     << ret << ": Could not parse carvemap #("
			     << ia << ", " << ia2 << ") for "
			     << "wedge #" << i << endl << endl;
			return ret;
		}
		ret = cm_infile.read(b1, ib, ib1);
		if(ret)
		{
			/* error occurred */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-6, ret);
			cerr << "[random_carver_t::export_chunks]\tError "
			     << ret << ": Could not parse carvemap #("
			     << ib << ", " << ib1 << ") for "
			     << "wedge #" << i << endl << endl;
			return ret;
		}
		ret = cm_infile.read(b2, ib, ib2);
		if(ret)
		{
			/* error occurred */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-7, ret);
			cerr << "[random_carver_t::export_chunks]\tError "
			     << ret << ": Could not parse carvemap #("
			     << ib << ", " << ib2 << ") for "
			     << "wedge #" << i << endl << endl;
			return ret;
		}
		
		/* prepare wedge information */
		w.init(&a1, &a2, &b1, &b2, wedge_infile.carving_buf()); 

		/* prepare the chunker with the info for this wedge */
		vals.resize(1);
		vals[0].set(i);
		chunker.set(&w, vals);
		
		/* carve this wedge with the chunk exporter */
		ret = this->tree.insert(chunker);
		if(ret)
		{
			/* error occurred during tree growth */
			progbar.clear();
			cm_infile.close();
			wedge_infile.close();
			chunker.close(this->tree);
			ret = PROPEGATE_ERROR(-8, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Unable to carve wedge #" << i << " into "
			     << "the tree, Error " << ret << endl;
			return ret;
		}
	}

	/* inform user that processing is finished */
	progbar.clear();
	cm_infile.close();
	wedge_infile.close();
	toc(clk, "Chunking wedges");

	/* close this chunker, which will finish exporting all files */
	tic(clk);
	chunker.close(this->tree);
	toc(clk, "Exporting chunk list");

	/* success */
	return 0;
}
		
int random_carver_t::carve_all_chunks(const string& cmfile,
                                      const string& wedgefile,
                                      const string& chunklist)
{
	Eigen::Vector3d treecenter;
	chunk::chunklist_reader_t chunk_infile;
	cm_io::reader_t cm_infile;
	wedge::reader_t wedge_infile;
	progress_bar_t progbar;
	size_t i, num_chunks;
	string chunkfile;
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
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::carve_all_chunks]\tError "
		     << ret << ": Unable to read chunklist file: "
		     << chunklist << endl;
		return ret;
	}
	
	/* prepare tree for importing chunks.  This will
	 * clear any existing data from tree. */
	treecenter << chunk_infile.center_x(),
	              chunk_infile.center_y(),
	              chunk_infile.center_z();
	this->tree.set(treecenter, chunk_infile.halfwidth(),
	               this->tree.get_resolution());

	/* open the carve map file for reading */
	ret = cm_infile.open(cmfile);
	if(ret)
	{
		chunk_infile.close();
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[random_carver_t::carve_all_chunks]\tError "
		     << ret << ": Unable to read carvemap file: "
		     << cmfile << endl << endl;
		return ret;
	}

	/* open the wedge file for reading.  The indices in the chunk
	 * file should reference the wedges in this wedge file. */
	ret = wedge_infile.open(wedgefile);
	if(ret)
	{
		/* unable to parse */
		chunk_infile.close();
		cm_infile.close();
		ret = PROPEGATE_ERROR(-3, ret);
		cerr << "[random_carver_t::carve_all_chunks]\tError "
		     << ret << ": Unable to read wedge file: "
		     << wedgefile << endl;
		return ret;
	}
	toc(clk, "Parsing files for carving");

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
			chunk_infile.close();
			cm_infile.close();
			wedge_infile.close();
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "Unable to get uuid for chunk #" << i
			     << ", Error " << ret << endl << endl;
			return ret;
		}

		/* process this chunk */
		ret = this->carve_chunk(cm_infile, wedge_infile,
					chunkfile, tp); 
		if(ret)
		{
			/* report error and continue */
			ret = PROPEGATE_ERROR(-5, ret);
			progbar.clear();
			chunk_infile.close();
			cm_infile.close();
			wedge_infile.close();
			cerr << "[random_carver_t::carve_all_chunks]\t"
			     << "Error " << ret << ": Unable to process "
			     << "chunk #" << i << ": " << chunkfile
			     << endl << endl;
			return ret;
		}

		/* make sure thread pool isn't overfull, just store
		 * enough to be actively processing on all threads,
		 * and have enough scheduled in the queue to refill
		 * those threads. */
		tp.wait(this->num_threads + this->num_threads);
	}

	/* wait for all threads to finish */
	tp.wait();

	/* attempt to fully simplify the entire tree */
	this->tree.get_root()->simplify_recur();
	
	/* clean up */
	chunk_infile.close();
	cm_infile.close();
	wedge_infile.close();
	progbar.clear();
	toc(clk, "Processing all chunks");

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
		
int random_carver_t::carve_chunk(cm_io::reader_t& carvemaps,
			wedge::reader_t& wedges,
			const string& chunkfile,
			boost::threadpool::pool& tp)
{
	Eigen::Vector3d chunkcenter;
	set<chunk::point_index_t> inds;
	chunk::chunk_reader_t infile;
	octnode_t* chunknode;
	unsigned int chunkdepth;
	int ret;

	/* open this chunk file for reading */
	ret = infile.open(chunkfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* import the indices into index set.  Since the
	 * set is sorted, the indices will be ordered
	 * with sensor-major, then frame, then point-minor
	 * ordering, which allows us to carve everything
	 * efficiently while only preparing each frame
	 * once. */
	infile.get_all(inds);

	/* check if file had duplicate indices */
	if(inds.size() != infile.num_points())
	{	
		/* error occurred */
		cerr << "[random_carver_t::carve_chunk]\tInfile had "
		     << infile.num_points() << " points but only " 
		     << inds.size()
		     << " were unique." << endl << endl;
		infile.close();
		return PROPEGATE_ERROR(-2, ret);
	}

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
			chunknode, inds, boost::ref(carvemaps),
			boost::ref(wedges),
			chunkdepth, false));
	}
	else
	{
		/* since we're not going to use multiple threads, don't
		 * bother using the threadpool at all, and instead just
		 * process the chunk using a direct function call */
		ret = random_carver_t::carve_node(chunknode, inds,
				carvemaps, wedges, chunkdepth, true);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[random_carver_t::carve_chunk]\tError "
			     << ret << ": Unable to carve node"
			     << endl << endl;
			return ret;
		}
	}

	/* clean up */
	return 0;
}
		
int random_carver_t::carve_node(octnode_t* chunknode,
			set<chunk::point_index_t> inds,
			cm_io::reader_t& carvemaps,
			wedge::reader_t& wedges,
			unsigned int maxdepth, bool verbose)
{
	set<chunk::point_index_t>::iterator it;
	unsigned int ia, ib; /* frame indices */
	unsigned int ia1, ia2, ib1, ib2; /* indices of imported carvemaps */
	carve_map_t a1, a2, b1, b2; /* imported carve maps */
	carve_wedge_t w;
	unsigned int i, n;
	progress_bar_t progbar;
	int ret;

	/* prepare a progress bar if running in verbose mode */
	i = 0;
	n = inds.size();
	if(verbose)
	{
		/* this will be a secondary progress bar, so
		 * pad the name so it lines up with the over-arching
		 * progress bar */
		progbar.set_name("     Current node");
		progbar.set_color(progress_bar_t::PURPLE);
	}

	/* iterate over the indices referenced in this chunk */
	for(it = inds.begin(); it != inds.end(); it++)
	{
		/* optionally show progress to user */
		if(verbose)
			progbar.update(i++, n);

		/* parse current wedge */
		ret = wedges.get(ia, ia1, ia2, ib, ib1, ib2,
				it->wedge_index);
		if(ret)
		{
			/* invalid index */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[random_carver_t::carve_node]\tError "
			     << ret << ": Unable to get wedge #"
			     << it->wedge_index << " from wedge file"
			     << endl << endl;
			return ret;
		}

		/* prepare the carve maps to be used for this wedge */
		ret = carvemaps.read(a1, ia, ia1);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[random_carver_t::carve_node]\tError "
			     << ret << ": Could not parse carvemap for "
			     << "wedge #" << it->wedge_index 
			     << endl << endl;
			return ret;
		}
		ret = carvemaps.read(a2, ia, ia2);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[random_carver_t::carve_node]\tError "
			     << ret << ": Could not parse carvemap for "
			     << "wedge #" << it->wedge_index 
			     << endl << endl;
			return ret;
		}
		ret = carvemaps.read(b1, ib, ib1);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[random_carver_t::carve_node]\tError "
			     << ret << ": Could not parse carvemap for "
			     << "wedge #" << it->wedge_index 
			     << endl << endl;
			return ret;
		}
		ret = carvemaps.read(b2, ib, ib2);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[random_carver_t::carve_node]\tError "
			     << ret << ": Could not parse carvemap for "
			     << "wedge #" << it->wedge_index 
			     << endl << endl;
			return ret;
		}
		
		/* prepare wedge information */
		w.init(&a1, &a2, &b1, &b2, wedges.carving_buf()); 

		/* carve the referenced wedge only in the domain of the
		 * given node */
		chunknode->insert(w, maxdepth);
	}

	/* simplify this chunk now that it is fully carved */
	chunknode->simplify_recur();

	/* success */
	return 0;
}
