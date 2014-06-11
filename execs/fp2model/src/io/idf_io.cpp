#include "idf_io.h"
#include "../structs/building_model.h"
#include "../structs/window.h"
#include "../process/subdivide_room.h"
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

/**
 * @file   idf_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Exports building models to .idf formatted files
 *
 * @section DESCRIPTION
 *
 * This file represents functions that will
 * generate Input Data Files (IDF) for the EnergyPlus
 * simulation framework.
 *
 * The primary consumer of these functions is
 * EnergyPlus, a building energy simulation tool.
 */

using namespace std;
using namespace fp;

/*-------------------- DEFINES ----------------------*/

#define MAX_VERTS_PER_SURFACE 120 /* IDF assumes 120 vertices at most */

/*---------------- FUNCTION BODIES ------------------*/

/* write a .idf file for energy plus */
int writeidf(const string& filename, const building_model_t& bm,
             bool verbose)
{
	ofstream outfile;
	int i, n;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write basic info about the model
	 * (this doesn't change between models) */
	writeversion(outfile);
	if(verbose)
	{
		/* this information is not strictly necessary */
		writebuilding(outfile, filename);
		writetimestep(outfile);
		writesimulationcontrol(outfile);
		writelocation(outfile);
	}

	/* these materials and constructions are used to define the
	 * properties of surfaces exported to the file */
	writecommonmats(outfile);
	writecommonconstructions(outfile);

	/* more optional paramters */
	if(verbose)
	{
		/* these optional fields specify HVAC and scheduling
		 * information in the building */
		writedefaultschedule(outfile);
		writethermostat(outfile,
			"Constant Setpoint Thermostat", 20, 25); 
		writedictionary(outfile);
	}

	/* compute the geometry for each room, and write to file */
	n = bm.floorplan.rooms.size();
	for(i = 0; i < n; i++)
	{
		/* write model for the i'th room */
		writeroom(outfile, bm, bm.floorplan.rooms[i]);
	}

	/* write footer information of this file */
	if(verbose)
		writefooter(outfile);

	/* clean up */
	outfile.close();
	return 0;
}

/*-------------- geometry helper functions -------------------*/

void writeroom(ofstream& outfile, const building_model_t& bm,
               const fp::room_t& r)
{
	stringstream zonename;

	/* name this zone */
	zonename << "Room_" << (1+r.ind);

	/* create a zone for this room */
	writesection(outfile, "ROOM GEOMETRY");
	outfile << "  Zone," << endl
	        << "    " << zonename.str() << ", !- Name" << endl
		<< "    0.0,           !- Direction of Relative "
		<< "North {deg}" << endl
		<< "    0.0,           !- X Origin {m}" << endl
		<< "    0.0,           !- Y Origin {m}" << endl
		<< "    0.0,           !- Z Origin {m}" << endl
		<< "    ,              !- Type" << endl
		<< "    1,             !- Multiplier" << endl
		<< "    autocalculate, !- Ceiling Height {m}" << endl
		<< "    autocalculate; !- volume {m3}" << endl << endl;

	/* write floor and ceiling geometry for room */
	writefloorandceiling(outfile, bm, r, zonename.str());

	/* write wall geometry for room */
	writewalls(outfile, bm, r, zonename.str());
}

void writefloorandceiling(ofstream& outfile, const building_model_t& bm,
                          const fp::room_t& r, const string& zonename,
                          unsigned int surfnum)
{
	vector<vector<int> > boundary_list;
	unsigned int i, n;
	stringstream floorname, ceilname;
	fp::room_t ra, rb;

	/* compute the edges of this room, which will become walls */
	bm.floorplan.compute_oriented_boundary(boundary_list, r.tris);

	/* check if we can export this boundary directly */
	if(boundary_list.size() == 0)
		return; /* don't do anything, no triangles */
	if(boundary_list.size() > 1 
			|| boundary_list[0].size() > MAX_VERTS_PER_SURFACE)
	{
		/* cannot yet export this area, must subdivide */
		bisect_room(ra, rb, r, bm.floorplan);
		
		/* now that the area has been subdivided, it can be
		 * exported recursively */
		writefloorandceiling(outfile,bm,ra,zonename,2*surfnum);
		writefloorandceiling(outfile,bm,rb,zonename,2*surfnum+1);

		/* we've successfully exported the subsurfaces, so 
		 * don't bother with this one anymore */
		return;
	}	

	/* create a floor for this room */
	floorname << zonename << ":Flr" << surfnum;
	n = boundary_list[0].size();
	outfile << "  BuildingSurface:Detailed," << endl
	        << "    " << floorname.str() << ", !- Name" << endl
		<< "    Floor,         !- Surface Type" << endl
		<< "    Interior Floor,!- Construction Name" << endl
		<< "    " << zonename << ", !- Zone Name" << endl
		<< "    Surface,       !- Outside Boundary Condition"
			<< endl
		<< "    , !- Outside Boundary Condition Object" << endl
		<< "    NoSun,         !- Sun Exposure" << endl
		<< "    NoWind,        !- Wind Exposure" << endl
		<< "    1.0,           !- View Factor to Ground" << endl
		<< "    " << n << ",   !- Number of Vertices" << endl;

	/* define geometry of floor */
	for(i = 0; i < n; i++)
	{
		/* add to floor */
		outfile << "    "
		        << bm.floorplan.verts[boundary_list[0][i]].x << "," 
		        << bm.floorplan.verts[boundary_list[0][i]].y << ","
		        << bm.floorplan.verts[boundary_list[0][i]].min_z
			<< (i == n-1 ? ";" : ",")
			<< "   !- Vertex " << i << " {m}" << endl;
	}
	outfile << endl;
	
	/* create a ceiling for this room */
	ceilname << zonename << ":Ceil" << surfnum;
	outfile << "  BuildingSurface:Detailed," << endl
	        << "    " << ceilname.str() << ", !- Name" << endl
		<< "    Ceiling,         !- Surface Type" << endl
		<< "    Interior Ceiling,!- Construction Name" << endl
		<< "    " << zonename << ", !- Zone Name" << endl
		<< "    Outdoors,         !- Outside Boundary Condition"
			<< endl
		<< "    , !- Outside Boundary Condition Object" << endl
		<< "    NoSun,           !- Sun Exposure" << endl
		<< "    NoWind,          !- Wind Exposure" << endl
		<< "    0,             !- View Factor to Ground" << endl
		<< "    " << n << ",     !- Number of Vertices" << endl;

	/* define geometry of ceiling */
	for(i = n; i >= 1; i--)
	{
		/* add to ceiling */
		outfile << "    "
		   << bm.floorplan.verts[boundary_list[0][i-1]].x << "," 
		   << bm.floorplan.verts[boundary_list[0][i-1]].y << ","
		   << bm.floorplan.verts[boundary_list[0][i-1]].max_z
		   << (i == 1 ? ";" : ",")
		   << "   !- Vertex " << (n-i) << " {m}" << endl;
	}
	outfile << endl;
}

void writewalls(ofstream& outfile, const building_model_t& bm,
                const fp::room_t& r, const string& zonename)
{
	vector<window_t> wins;
	vector<fp::edge_t> edge_list;
	stringstream wallname_ss;
	string wallname;
	unsigned int i, j, n, num_wins;
	double wx[NUM_VERTS_PER_RECT];
	double wy[NUM_VERTS_PER_RECT];
	double wz[NUM_VERTS_PER_RECT];
	double px, py;

	/* get boundary of this room */
	bm.floorplan.compute_edges_for_room(edge_list, r.ind);

	/* export each edge as a separate wall */
	n = edge_list.size();
	for(i = 0; i < n; i++)
	{
		/* prepare attributes of this wall */
		wallname_ss.str("");
		wallname_ss << zonename << ":Wall" << (i+1);
		wallname = wallname_ss.str();

		/* check if this wall has windows */
		wins.clear();
		bm.windows.get_windows_for(edge_list[i], wins);

		/* check trivial case */
		num_wins = wins.size();
		if(num_wins == 0)
		{
			/* write a standard wall */
			writesubwall(outfile,
				bm.floorplan.verts[edge_list[i].verts[0]].x,
				bm.floorplan.verts[edge_list[i].verts[0]].y,
				bm.floorplan.verts[edge_list[i].verts[1]].x,
				bm.floorplan.verts[edge_list[i].verts[1]].y,
				r.min_z, r.max_z, wallname, zonename);
			continue;
		}

		/* since this wall contains windows, we need to break
		 * it up into multiple surfaces, since IDF cannot handle
		 * surfaces with holes in them.
		 *
		 * In order to efficiently split the wall, we want to
		 * sort the windows by position.  NOTE: this process
		 * assumes the windows do not overlap horizontally, which
		 * would occur if one window was above another. */
		sort(wins.begin(), wins.end());

		/* write portion of wall before first window */
		wallname_ss.str("");
		wallname_ss << wallname << "_sub" << 0;
		wins[0].get_world_coords(wx,wy,wz,bm.floorplan);
		writesubwall(outfile,
			bm.floorplan.verts[edge_list[i].verts[0]].x,
			bm.floorplan.verts[edge_list[i].verts[0]].y,
			wx[2],wy[2],r.min_z,r.max_z,
			wallname_ss.str(), zonename);

		/* write portions of wall around each window */
		for(j = 0; j < num_wins; j++)
		{
			/* the j'th window geometry is already
			 * loaded into the arrays wx, wy, wz */

			/* write portion of wall below window */
			wallname_ss.str("");
			wallname_ss << wallname << "_sub" << (1+3*j);
			writesubwall(outfile,wx[0],wy[0],wx[2],wy[2],
					r.min_z,wz[0],
					wallname_ss.str(), zonename);

			/* write window geometry */
			writewindow(outfile,wx[0],wy[0],wx[2],wy[2],
					wz[0],wz[1],j,wallname);

			/* write portion of wall above window */
			wallname_ss.str("");
			wallname_ss << wallname << "_sub" << (2+3*j);
			writesubwall(outfile,wx[0],wy[0],wx[2],wy[2],
					wz[1],r.max_z,
					wallname_ss.str(), zonename);

			/* write portion of wall between this window
			 * and the next window */
			wallname_ss.str("");
			wallname_ss << wallname << "_sub" << (3+3*j);
			if(j+1 < num_wins)
			{
				/* get geometry of next window and save
				 * position of the current window */
				px = wx[2];
				py = wy[2];
				wins[j+1].get_world_coords(wx,wy,wz,
						bm.floorplan);
				
				/* export subwall between windows */
				writesubwall(outfile,px,py,wx[0],wy[0],
						r.min_z,r.max_z,
						wallname_ss.str(),
						zonename);
			}
			else
			{
				/* export remainder of wall */
				writesubwall(outfile, wx[2], wy[2],
					bm.floorplan.verts[
						edge_list[i].verts[0]].x,
					bm.floorplan.verts[
						edge_list[i].verts[0]].y,
					r.min_z, r.max_z, wallname_ss.str(),
					zonename);
			}
		}
	}
}

void writesubwall(ofstream& outfile, double x1, double y1,
		double x2, double y2, double min_z, double max_z,
		const string& name, const string& zonename)
{
	/* NOTE:  the 'Outside Boundary Condition Object'
	 * represents the surface on the other side of the wall
	 * from this surface.  Generally, these surfaces should be
	 * explicitly linked in the file in order to ensure proper
	 * thermal simulation */

	/* make a wall */
	outfile << "  BuildingSurface:Detailed," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    Wall,          !- Surface Type" << endl
		<< "    Interior Wall, !- Construction Name" << endl
		<< "    " << zonename << ", !- Zone name" << endl
		<< "    Surface,       !- Outside Boundary Condition" 
			<< endl
		<< "    , !- Outside Boundary Condition Object" << endl 
		<< "    NoSun,         !- Sun Exposure" << endl
		<< "    NoWind,        !- Wind Exposure" << endl
		<< "    ,              !- View Factor to Ground" << endl
		<< "    4,             !- Number of Vertices" << endl	
		<< "    " << x1 << "," << y1 << "," << min_z
			<< ", !- Vertex 1, lower right {m}" << endl
		<< "    " << x1 << "," << y1 << "," << max_z
			<< ", !- Vertex 2, upper right {m}" << endl
		<< "    " << x2 << "," << y2 << "," << max_z
			<< ", !- Vertex 3, upper left {m}" << endl
		<< "    " << x2 << "," << y2 << "," << min_z
			<< "; !- Vertex 4, lower left {m}" << endl
		<< endl;
}

void writewindow(ofstream& outfile, double x1, double y1,
		double x2, double y2, double min_z, double max_z,
		unsigned int num, const string& wallname)
{
	/* make a wall */
	outfile << "  FenestrationSurface:Detailed," << endl
	        << "    " << wallname << ":Window" << num 
			<< ", !- Name" << endl
		<< "    Window,          !- Surface Type" << endl
		<< "    Exterior Window, !- Construction Name" << endl
		<< "    " << wallname << ", !- Building Surface Name"<< endl
		<< "    , !- Outside Boundary Condition Object" << endl 
		<< "    , !- View Factor to Ground" << endl
		<< "    , !- Shading Control Name" << endl
		<< "    , !- Frame and Divider Name" << endl
		<< "    , !- Multiplier" << endl
		<< "    4, !- Number of Vertices" << endl	
		<< "    " << x1 << "," << y1 << "," << min_z
			<< ", !- Vertex 1, lower right {m}" << endl
		<< "    " << x1 << "," << y1 << "," << max_z
			<< ", !- Vertex 2, upper right {m}" << endl
		<< "    " << x2 << "," << y2 << "," << max_z
			<< ", !- Vertex 3, upper left {m}" << endl
		<< "    " << x2 << "," << y2 << "," << min_z
			<< "; !- Vertex 4, lower left {m}" << endl
		<< endl;
}

/**************** implementation of helper functions ******************/

/* writes the version of Energy Plus that is supported */
void writeversion(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: VERSION");
	outfile << "  Version,7.2;" << endl << endl;
}

/* writes information about the building described in a model */
void writebuilding(ofstream& outfile, const string& name)
{
	/* write building information */
	writesection(outfile, "ALL OBJECTS IN CLASS: BUILDING");
	outfile << "  Building," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    0.0, !- North Axis {deg}" << endl
		<< "    City, !- Terrain" << endl
		<< "    0.04, !- Loads Convergence Tolerance Value" << endl
		<< "    0.4, !- Temperature Convergence Tolerance Value "
		<< "{deltaC}" << endl
		<< "    FullInteriorAndExterior, !- Solar Distribution"
		<< endl
		<< "    25; !- Maximum Number of Warmup Days"
		<< endl << endl;
}

/* writes the default simulation timestep for the file */
void writetimestep(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: TIMESTEP");
	outfile << "  Timestep,4;" << endl << endl;
}

/* writes the simulation controls for the .idf file */
void writesimulationcontrol(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: SIMULATIONCONTROL");
	outfile << "  SimulationControl," << endl
	        << "    No,   !- Do Zone Sizing Calculation" << endl
		<< "    No,   !- Do System Sizing Calculation" << endl
		<< "    No,   !- Do Plant Sizing Calculation" << endl
		<< "    Yes,  !- Run Simulation for Sizing Periods" << endl
		<< "    Yes;  !- Run Simulation for Weather File "
		<< "Run Periods" << endl << endl;
}

/* writes the building location.  Since this program is unable to
 * deduce the location, a default value is specified (residing in
 * Chicago) */
void writelocation(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: LOCATION");
	outfile << "  Site:Location," << endl
	        << "    CHICAGO_IL_USA TMY2-94846, !- Name" << endl
		<< "    41.78,   !- Latitude  {deg}" << endl
		<< "    -87.75,  !- Longitude {deg}" << endl
		<< "    -6.0,    !- Time Zone {hr}" << endl
		<< "    190.00;  !- Elevation {m}" << endl
		<< endl
		<< "  GlobalGeometryRules," << endl
		<< "    UpperLeftCorner,    "
		<< "!- Starting Vertex Position" << endl
		<< "    Counterclockwise,   "
		<< "!- Vertex Entry Direction" << endl
		<< "    Absolute;           "
		<< "!- Coordinate System" << endl << endl;
}

/* writes common materials to an .idf file */
void writecommonmats(ofstream& outfile)
{
	/* write solids */
	writesection(outfile, "ALL OBJECTS IN CLASS: MATERIAL");
	writemat(outfile, "F08 Metal surface", "Smooth", 0.0008, 45.28,
			7824, 500, 0.9, 0, 0);
	writemat(outfile, "I01 25mm insulation board", "MediumRough",
			0.0254, 0.03, 43, 1210, 0.9, 0, 0);
	writemat(outfile, "I02 50mm insulation board", "MediumRough",
			0.0508, 0.03, 43, 1210, 0.9, 0, 0);
	writemat(outfile, "G01a 19mm gypsum board", "MediumSmooth",
			0.019, 0.16, 800, 1090, 0.7, 0, 0);
	writemat(outfile, "M11 100mm lightweight concrete", "MediumRough",
			0.1016, 0.53, 1280, 840, 0.6, 0, 0);
	writemat(outfile, "F16 Acoustic tile", "MediumSmooth",
			0.0191, 0.06, 368, 590, 0.9, 0, 0);
	writemat(outfile, "M01 100mm brick", "MediumRough", 0.1016,
			0.89, 1920, 790, 0.5, 0, 0);
	writemat(outfile, "M15 200mm heavyweight concrete", "MediumRough",
			0.2032, 1.95, 2240, 900, 0.6, 0, 0);
	writemat(outfile, "M05 200mm concrete block", "MediumRough",
			0.2032, 1.11, 800, 920, 0.6, 0, 0);
	writemat(outfile, "G05 25mm wood", "MediumSmooth",
			0.0254, 0.15, 608, 1630, 0.7, 0, 0);
	writemat(outfile, "B4 - 3 IN INSULATION", "VeryRough",
			0.07, 0.04, 32.03, 830.0, 0.9, 0.5, 0.5);
	writemat(outfile, "PVModule(glass)", "Smooth", 0.007, 0.78,
			2700.0, 840.0, 0.84, 0.92, 0.92);
	writemat(outfile, "E6 - 1 / 2 IN GYP SHEATHING BOARD", "Smooth",
			0.01, 0.16, 784.9, 830.0, 0.9, 0.92, 0.92);
	writemat(outfile, "ROOFING - BUILT UP ROOFING - 3 / 8 IN",
			"VeryRough", 0.00949999, 0.16, 1121.29,
			1460.0, 0.9, 0.7, 0.7);
	writemat(outfile, "INS-Roof-R_15", "MediumRough", 0.1295,
			0.049, 265, 836.8, 0.9, 0.7, 0.7);
	writemat(outfile, "MAT-METAL", "MediumSmooth", 0.001524, 45.006,
			7680, 418.4, 0.9, 0.7, 0.7);
	writemat(outfile, "IN35", "Rough", 0.050799999, 0.035, 29.0,
			1213.0, 0.9, 0.5, 0.5);

	/* write air gaps */
	writesection(outfile, "ALL OBJECTS IN CLASS: MATERIAL:AIRGAP");
	writeairgap(outfile, "F04 Wall air space resistance", 0.15);
	writeairgap(outfile, "F05 Ceiling air space resistance", 0.18);

	/* write window glazing materials */
	writesection(outfile, 
		"ALL OBJECTS IN CLASS: WINDOWMATERIAL:GLAZING");
	writewindowglazing(outfile, "Clear 3mm", "SpectralAverage", "",
			0.003, 0.837, 0.075, 0.075, 0.075, 0.898,
			0.081, 0.081, 0.84, 0.84, 0.9);
	
	/* write window gas materials */
	writesection(outfile, "ALL OBJECTS IN CLASS: WINDOWMATERIAL:GAS");
	writewindowgas(outfile, "Air 13mm", "Air", 0.0127);
}

/* will write several common constructions to the file being generated */
void writecommonconstructions(ofstream& outfile)
{
	/* define material names */
	const string air = "Air 13mm";
	const string clear = "Clear 3mm";
	const string F04 = "F04 Wall air space resistance";
	const string F05 = "F05 Ceiling air space resistance";
	const string F08 = "F08 Metal Surface";
	const string F16 = "F16 Acoustic tile";
	const string G01a = "G01a 19mm gypsum board";
	const string G05 = "G05 25mm wood";
	const string I01 = "I01 25mm insulation board";
	const string I02 = "I02 50mm insulation board";
	const string I05 = "I05 50mm insulation board";
	const string M01 = "M01 100mm brick";
	const string M11 = "M11 100mm lightweight concrete";
	const string M15 = "M15 200mm heavyweight concrete";
	
	/* define layerings */
	const string exterior_floor[] = {I05, M15}; 
	const string interior_floor[] = {F16, F05, M11};
	const string exterior_wall[] = {M01, M15, I02, F04, G01a};
	const string interior_wall[] = {G01a, F04, G01a};
	const string exterior_roof[] = {M11, F05, F16};
	const string interior_ceiling[] = {M11, F05, F16};
	const string exterior_window[] = {clear, air, clear};
	const string interior_window[] = {clear};
	const string exterior_door[] = {F08, I01};
	const string interior_door[] = {G05};

	/* start section, and write materials */
	writesection(outfile, "ALL OBJECTS IN CLASS: CONSTRUCTION");
	writeconstruction(outfile, "Exterior Floor", exterior_floor, 2);
	writeconstruction(outfile, "Interior Floor", interior_floor, 3);
	writeconstruction(outfile, "Exterior Wall", exterior_wall, 5);
	writeconstruction(outfile, "Interior Wall", interior_wall, 3);
	writeconstruction(outfile, "Exterior Roof", exterior_roof, 3);
	writeconstruction(outfile, "Interior Ceiling", interior_ceiling, 3);
	writeconstruction(outfile, "Exterior Window", exterior_window, 3);
	writeconstruction(outfile, "Interior Window", interior_window, 1);
	writeconstruction(outfile, "Exterior Door", exterior_door, 2);
	writeconstruction(outfile, "Interior Door", interior_door, 1);
}

/* will write a default schedule for this building model */
void writedefaultschedule(ofstream& outfile)
{
	/* write the schedule type limits */
	writesection(outfile, "ALL OBJECTS IN CLASS: SCHEDULETYPELIMITS");
	outfile << "  ScheduleTypeLimits," << endl 
	        << "    Any Number; !- Name" << endl << endl;
	writescheduletypelimit(outfile, "Fraction",0.0,1.0,"CONTINUOUS");
	writescheduletypelimit(outfile, "Temperature",-60,200,"CONTINUOUS");
	writescheduletypelimit(outfile, "On/Off",0,1,"DISCRETE");
	writescheduletypelimit(outfile, "Control Type",0,4,"DISCRETE");
	writescheduletypelimit(outfile, "Humidity",10,90,"CONTINUOUS");
	outfile << "  ScheduleTypeLimits," << endl
	        << "    Number; !- Name" << endl << endl;

	/* write Run Period Control, and holidays */
	writesection(outfile, 
		"ALL OBJECTS IN CLASS: RUNPERIODCONTROL:SPECIALDAYS");
	writeholiday(outfile, "New Years Day", "January 1", 1, "Holiday"); 
	writeholiday(outfile, "Veterans Day", "November 11", 1, "Holiday"); 
	writeholiday(outfile, "Christmas", "December 25", 1, "Holiday"); 
	writeholiday(outfile, "Independence Day", "July 4", 1, "Holiday"); 
	writeholiday(outfile,"MLK Day","3rd Monday of January",1,"Holiday");
	writeholiday(outfile, "Presidents Day", "3rd Monday of February",
		1, "Holiday");
	writeholiday(outfile, "Memorial Day", "Last Monday of May",
		1, "Holiday");
	writeholiday(outfile, "Labor Day", "1st Monday of September",
		1, "Holiday");
	writeholiday(outfile, "Columbus Day", "2nd Monday in October", 
		1, "Holiday");
	writeholiday(outfile, "Thanksgiving", "4th Thursday in November", 
		1, "Holiday");

	/* specify daylight savings time */
	outfile << "  RunPeriodControl:DaylightSavingTime," << endl
	        << "    2nd Sunday in March, !- Start Date" << endl
		<< "    1st Sunday in November; !- End Date" << endl 
		<< endl;
}

void writedictionary(ofstream& outfile)
{
	writesection(outfile, 
		"ALL OBJECTS IN CLASS: OUTPUT:VARIABLEDICTIONARY");
	outfile << "  Output:VariableDictionary,IDF;" << endl << endl;
}

void writefooter(ofstream& outfile)
{
	writesection(outfile, "OUTPUT VARIABLES");
	outfile << "  Output:Meter,Photovoltaic:ElectricityProduced,"
	        << "runperiod;" << endl << endl;
	outfile << "  OutputControl:Table:Style," << endl
	        << "    TabAndHTML;         !- Column Separator" << endl
		<< endl;
	outfile << "  Output:Table:SummaryReports," << endl
	        << "    AllSummary;         !- Report 1 Name" << endl
		<< endl;
}

/* writes a full-line comment to an .idf file */
void writesection(ofstream& outfile, const string& text)
{
	outfile << " !- === " << text << " ===" << endl << endl;
}

/* writes one material to an .idf file with the specified properties */
void writemat(ofstream& outfile, const string& name, 
		const string& roughness, 
		double thickness, double conductivity, double density,
		double specific_heat, double thermal_absorptance,
		double solar_absorptance, double visible_absorptance)
{
	outfile << "  Material," << endl
		<< "    " << name << ", !- Name" << endl
		<< "    " << roughness << ", !- Roughness" << endl
		<< "    " << thickness << ", !- Thickness {m}" << endl
		<< "    " << conductivity << ", !- Conductivity {W/m-K}" 
		<< endl
		<< "    " << density << ", !- Density {kg/m3}" << endl
		<< "    " << specific_heat << ", !- Specific Heat {J/kg-K}"
		<< endl
		<< "    " << thermal_absorptance 
		<< ", !- Thermal Absorptance" << endl
		<< "    " << solar_absorptance << ", !- Solar Absorptance"
		<< endl
		<< "    " << visible_absorptance 
		<< "; !- Visible Absorptance" << endl << endl;
}

/* writes one material for gas (a.k.a. air) */
void writeairgap(ofstream& outfile, const string& name, 
		double thermal_resistance)
{
	outfile << "  Material:AirGap," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    " << thermal_resistance
		<< "; !- Thermal Resistance {m2-K/W}" << endl << endl;
}

/* writes window glazing to file, which describes the reflectance
 * and transmittance properties of windows within this building */
void writewindowglazing(ofstream& outfile, const string& name,
		const string& optical_data_type, const string& dataset_name,
		double thickness, double solar_transmittance,
		double front_solar_reflectance, 
		double back_solar_reflectance,
		double visible_transmittance,
		double front_visible_reflectance,
		double back_visible_reflectance,
		double infrared_transmittance,
		double front_infrared_emissivity,
		double back_infrared_emissivity,
		double conductivity)
{
	outfile << "  WindowMaterial:Glazing," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    " << optical_data_type << ", !- Optical Data Type"
		<< endl
		<< "    " << dataset_name << ", !- Data Set Name" << endl
		<< "    " << thickness << ", !- Thickness {m}" << endl
		<< "    " << solar_transmittance
		<< ", !- Solar Transmittance at Normal Incidence" << endl
		<< "    " << front_solar_reflectance
		<< ", !- Front Side Solar Reflectance at Normal Incidence"
		<< endl
		<< "    " << back_solar_reflectance
		<< ", !- Back Side Solar Reflectance at Normal Incidence"
		<< endl
		<< "    " << visible_transmittance
		<< ", !- Visible Transmittance at Normal Incidence"
		<< endl
		<< "    " << front_visible_reflectance
		<< ", !- Front Side Visible Reflectance at Normal Incidence"
		<< endl
		<< "    " << back_visible_reflectance
		<< ", !- Back Side Visible Reflectance at Normal Incidence"
		<< endl
		<< "    " << infrared_transmittance
		<< ", !- Infrared Transmittance at Normal Incidence"
		<< endl
		<< "    " << front_infrared_emissivity
		<< ", !- Front Side Infrared Hemispherical Emissivity"
		<< endl
		<< "    " << back_infrared_emissivity
		<< ", !- Back Side Infrared Hemispherical Emissivity"
		<< endl
		<< "    " << conductivity << "; !- Conductivity {W/m-K}"
		<< endl << endl;
}

/* writes window gas to .idf file, which describes the air gap within
 * a window. */
void writewindowgas(ofstream&outfile, const string& name, 
		const string& type, double thickness)
{
	outfile << "  WindowMaterial:Gas," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    " << type << ", !- Type" << endl
		<< "    " << thickness << "; !- Thickness {m}"
		<< endl << endl;
}

/* writes one construction to file.  This represents a set of materials,
 * layered, with the outer material described first, working inwards */
void writeconstruction(ofstream& outfile, const string& name, 
		const string* layers, int num_layers)
{
	int i;

	/* write name */
	outfile << "  Construction," << endl
	        << "    " << name << ", !- Name" << endl;
	
	/* write layers */
	for(i = 0; i < num_layers-1; i++)
		outfile << "    " << layers[i] << ", !- Layer " 
			<< (i+1) << endl;

	/* write last layer */
	outfile << "    " << layers[num_layers-1] 
		<< "; !- Inner Layer" << endl << endl;
}

/* write a schedule type limit, which specifies upper and lower bounds
 * for some value */
void writescheduletypelimit(ofstream& outfile, const string& name,
		double lower, double upper, const string& type)
{
	outfile << "  ScheduleTypeLimits,"
	        << "    " << name << ", !- Name" << endl
		<< "    " << lower << ", !- Lower Limit Value {A3}" << endl
		<< "    " << upper << ", !- Upper Limit Value {A3}" << endl
		<< "    " << type << "; !- Numeric Type" << endl << endl;
}

/* the building model can specify special days for scheduling purposes.
 * The following function will write one of them. */
void writeholiday(ofstream& outfile, const string& name, 
		const string& start, int duration, const string& type)
{
	outfile << "  RunPeriodControl:SpecialDays," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    " << start << ", !- Start Date" << endl
		<< "    " << duration << ", !- Duration {days}" << endl
		<< "    " << type << "; !- Special Day Type" << endl 
		<< endl;
}

/* specifies thermostat levels */
void writethermostat(ofstream& outfile, const string& name, 
		double heating_set, double cooling_set)
{
	writesection(outfile, "HVACTemplate:Thermostat");
	outfile << "  HVACTemplate:Thermostat," << endl
	        << "    " << name << ", !- Name" << endl
		<< "    , !- Heating Setpoint Schedule Name" << endl
		<< "    " << heating_set 
		<< ", !- Constant Heating Setpoint {C}" << endl
		<< "    , !- Cooling Setpoint Schedule Name" << endl
		<< "    " << cooling_set 
		<< "; !- Constant Cooling Setpoint {C}" << endl << endl;
}
