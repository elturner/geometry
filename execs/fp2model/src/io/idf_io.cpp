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
	writedefaultschedule(outfile);
	if(verbose)
	{
		/* these optional fields specify HVAC and scheduling
		 * information in the building */
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
	zonename << bm.level_name << "_Room_" << (1+r.ind);

	/* create a zone for this room */
	writesection(outfile, "ROOM GEOMETRY");
	outfile << "  Zone," << "\r\n"
	        << "    " << zonename.str() << ", !- Name" << "\r\n"
		<< "    0.0,           !- Direction of Relative "
		<< "North {deg}" << "\r\n"
		<< "    0.0,           !- X Origin {m}" << "\r\n"
		<< "    0.0,           !- Y Origin {m}" << "\r\n"
		<< "    0.0,           !- Z Origin {m}" << "\r\n"
		<< "    ,              !- Type" << "\r\n"
		<< "    1,             !- Multiplier" << "\r\n"
		<< "    autocalculate, !- Ceiling Height {m}" << "\r\n"
		<< "    autocalculate; !- volume {m3}" << "\r\n" << "\r\n";

	/* write ceiling lights, plug loads, and equipment information */
	writelightspeopleandplugloads(outfile, bm, r.ind, zonename.str());

	/* write floor and ceiling geometry for room */
	writefloorandceiling(outfile, bm, r, zonename.str());

	/* write wall geometry for room */
	writewalls(outfile, bm, r, zonename.str());
}

void writelightspeopleandplugloads(std::ostream& outfile, 
			const building_model_t& bm, 
			size_t ri, const string& zonename)
{
	double floorarea, watts;
	size_t people;

	/* compute the floor area for this room */
	floorarea = bm.floorplan.compute_room_area(ri);

	/* check if any people are defined */
	if(bm.people.size() > 0)
	{
		/* get the people count for this room */
		people = bm.people.get_room(ri);

		/* export stats */
		outfile << "  People," << "\r\n"
		        << "    " << zonename 
			<< " People 1, !- Name"<<"\r\n"
		        << "    " << zonename << ", !- Zone name" << "\r\n"
		        << "    OCCUPY-1, !- Schedule name" << "\r\n"
		        << "    people,   !- Calculation method" << "\r\n"
		        << "    " << people 
			<< ", !- Number of people"<<"\r\n"
		        << "    " << (people/floorarea) 
			<< ", !- people per area {person/m2}"<<"\r\n"
		        << "    ";
		if(people > 0)
			outfile << (floorarea/people);
		outfile << ", !- area / person {m2/person}" << "\r\n"
		        << "    0.3, !- Fraction Radiant " << "\r\n"
		        << "    , !- Sensible Heat Fraction" << "\r\n"
		        << "    ActSchd; !- Activity Level Schedule Name" 
			<< "\r\n" << "\r\n";
	}
	else
		people = 0;

	/* check if any lights are defined */
	if(bm.lights.size() > 0)
	{
		/* get the wattage for this room */
		watts = bm.lights.get_room(ri);

		/* export stats */
		outfile << "  Lights," << "\r\n"
		        << "    " << zonename << " Lights 1,  "
			<< "!- Name" << "\r\n"
		        << "    " << zonename << ",           "
			<< "!- Zone Name" << "\r\n"
		        << "    LIGHTS-1,   !- Schedule Name" << "\r\n"
			<< "    LightingLevel,    "
			<< "!- Design Level Calc Method" << "\r\n"
			<< "    " << watts << "   "
			<< "!- Design Level {Watts}" << "\r\n"
			<< "    " << (watts/floorarea) << "    "
			<< "!- Watts per floor area {W/m2}" << "\r\n"
			<< "    ";
		if(people > 0)
			outfile << (watts/people);
		outfile << ",    "
			<< "!- Watts per Person {w/person}" << "\r\n"
			<< "    0, !- Return Air Fraction" << "\r\n"
			<< "    0.42,   !- Fraction Radiant" << "\r\n"
			<< "    0.18,   !- Fraction Visible" << "\r\n"
			<< "    1,     !- Fraction Replaceable" << "\r\n"
			<< "    GeneralLights;    "
			<< "!- End-Use Subcategory" << "\r\n" << "\r\n";
	}

	/* check if any plug loads are defined */
	if(bm.plugloads.size() > 0)
	{
		/* get the wattage for this room */
		watts = bm.plugloads.get_room(ri);

		/* export stats */
		outfile << "  ElectricEquipment," << "\r\n"
		        << "    " << zonename << " ElecEq 1,  "
			<< "!- Name" << "\r\n"
		        << "    " << zonename << ",           "
			<< "!- Zone Name" << "\r\n"
		        << "    EQUIP-1,   !- Schedule Name" << "\r\n"
			<< "    EquipmentLevel,    "
			<< "!- Design Level Calc Method" << "\r\n"
			<< "    " << watts << ",   "
			<< "!- Design Level {Watts}" << "\r\n"
			<< "    " << (watts/floorarea) << ",    "
			<< "!- Watts per floor area {W/m2}" << "\r\n"
			<< "    ";
		if(people > 0)
			outfile << (watts/people);
		outfile << ",    "
			<< "!- Watts per Person {w/person}" << "\r\n"
			<< "    0,   !- Fraction Latent" << "\r\n"
			<< "    0.3, !- Fraction Radiant" << "\r\n"
			<< "    0;   !- Fraction Lost" << "\r\n" << "\r\n";
	}
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
	outfile << "  BuildingSurface:Detailed," << "\r\n"
	        << "    " << floorname.str() << ", !- Name" << "\r\n"
		<< "    Floor,         !- Surface Type" << "\r\n"
		<< "    Interior Floor,!- Construction Name" << "\r\n"
		<< "    " << zonename << ", !- Zone Name" << "\r\n"
		<< "    Surface,       !- Outside Boundary Condition"
			<< "\r\n"
		<< "    , !- Outside Boundary Condition Object" << "\r\n"
		<< "    NoSun,         !- Sun Exposure" << "\r\n"
		<< "    NoWind,        !- Wind Exposure" << "\r\n"
		<< "    Autocalculate, !- View Factor to Ground" << "\r\n"
		<< "    " << n << ",   !- Number of Vertices" << "\r\n";

	/* define geometry of floor */
	for(i = 0; i < n; i++)
	{
		/* add to floor */
		outfile << "    "
		        << bm.floorplan.verts[boundary_list[0][i]].x << "," 
		        << bm.floorplan.verts[boundary_list[0][i]].y << ","
		        << r.min_z
			<< (i == n-1 ? ";" : ",")
			<< "   !- Vertex " << i << " {m}" << "\r\n";
	}
	outfile << "\r\n";
	
	/* create a ceiling for this room */
	ceilname << zonename << ":Ceil" << surfnum;
	outfile << "  BuildingSurface:Detailed," << "\r\n"
	        << "    " << ceilname.str() << ", !- Name" << "\r\n"
		<< "    Ceiling,         !- Surface Type" << "\r\n"
		<< "    Interior Ceiling,!- Construction Name" << "\r\n"
		<< "    " << zonename << ", !- Zone Name" << "\r\n"
		<< "    Outdoors,         !- Outside Boundary Condition"
			<< "\r\n"
		<< "    , !- Outside Boundary Condition Object" << "\r\n"
		<< "    NoSun,           !- Sun Exposure" << "\r\n"
		<< "    NoWind,          !- Wind Exposure" << "\r\n"
		<< "    Autocalculate,   !- View Factor to Ground" << "\r\n"
		<< "    " << n << ",     !- Number of Vertices" << "\r\n";

	/* define geometry of ceiling */
	for(i = n; i >= 1; i--)
	{
		/* add to ceiling */
		outfile << "    "
		   << bm.floorplan.verts[boundary_list[0][i-1]].x << "," 
		   << bm.floorplan.verts[boundary_list[0][i-1]].y << ","
		   << r.max_z
		   << (i == 1 ? ";" : ",")
		   << "   !- Vertex " << (n-i) << " {m}" << "\r\n";
	}
	outfile << "\r\n";
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
		num_wins = wins.size();

		/* write a standard wall */
		writesubwall(outfile,
			bm.floorplan.verts[edge_list[i].verts[0]].x,
			bm.floorplan.verts[edge_list[i].verts[0]].y,
			bm.floorplan.verts[edge_list[i].verts[1]].x,
			bm.floorplan.verts[edge_list[i].verts[1]].y,
			r.min_z, r.max_z, wallname, zonename);

		/* write each window, overlapping the wall geometry */
		for(j = 0; j < num_wins; j++)
		{
			/* get window geometry */
			wins[j].get_world_coords(wx, wy, wz, bm.floorplan);

			/* write window geometry */
			writewindow(outfile,wx[0],wy[0],wx[2],wy[2],
					wz[0],wz[1],j,wallname);
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
	outfile << "  BuildingSurface:Detailed," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    Wall,          !- Surface Type" << "\r\n"
		<< "    Interior Wall, !- Construction Name" << "\r\n"
		<< "    " << zonename << ", !- Zone name" << "\r\n"
		<< "    Surface,       !- Outside Boundary Condition" 
			<< "\r\n"
		<< "    , !- Outside Boundary Condition Object" << "\r\n" 
		<< "    NoSun,         !- Sun Exposure" << "\r\n"
		<< "    NoWind,        !- Wind Exposure" << "\r\n"
		<< "    Autocalculate, !- View Factor to Ground" << "\r\n"
		<< "    4,             !- Number of Vertices" << "\r\n"	
		<< "    " << x1 << "," << y1 << "," << min_z
			<< ", !- Vertex 1, lower right {m}" << "\r\n"
		<< "    " << x1 << "," << y1 << "," << max_z
			<< ", !- Vertex 2, upper right {m}" << "\r\n"
		<< "    " << x2 << "," << y2 << "," << max_z
			<< ", !- Vertex 3, upper left {m}" << "\r\n"
		<< "    " << x2 << "," << y2 << "," << min_z
			<< "; !- Vertex 4, lower left {m}" << "\r\n"
		<< "\r\n";
}

void writewindow(ofstream& outfile, double x1, double y1,
		double x2, double y2, double min_z, double max_z,
		unsigned int num, const string& wallname)
{
	/* make a wall */
	outfile << "  FenestrationSurface:Detailed," << "\r\n"
	        << "    " << wallname << ":Window" << num 
			<< ", !- Name" << "\r\n"
		<< "    Window,          !- Surface Type" << "\r\n"
		<< "    Exterior Window, !- Construction Name" << "\r\n"
		<< "    " << wallname << ", !- Building Surface Name"<< "\r\n"
		<< "    , !- Outside Boundary Condition Object" << "\r\n" 
		<< "    Autocalculate, !- View Factor to Ground" << "\r\n"
		<< "    , !- Shading Control Name" << "\r\n"
		<< "    , !- Frame and Divider Name" << "\r\n"
		<< "    , !- Multiplier" << "\r\n"
		<< "    4, !- Number of Vertices" << "\r\n"	
		<< "    " << x1 << "," << y1 << "," << min_z
			<< ", !- Vertex 1, lower right {m}" << "\r\n"
		<< "    " << x1 << "," << y1 << "," << max_z
			<< ", !- Vertex 2, upper right {m}" << "\r\n"
		<< "    " << x2 << "," << y2 << "," << max_z
			<< ", !- Vertex 3, upper left {m}" << "\r\n"
		<< "    " << x2 << "," << y2 << "," << min_z
			<< "; !- Vertex 4, lower left {m}" << "\r\n"
		<< "\r\n";
}

/**************** implementation of helper functions ******************/

/* writes the version of Energy Plus that is supported */
void writeversion(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: VERSION");
	outfile << "  Version,8.0;" << "\r\n" << "\r\n";
}

/* writes information about the building described in a model */
void writebuilding(ofstream& outfile, const string& name)
{
	/* write building information */
	writesection(outfile, "ALL OBJECTS IN CLASS: BUILDING");
	outfile << "  Building," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    0.0, !- North Axis {deg}" << "\r\n"
		<< "    City, !- Terrain" << "\r\n"
		<< "    0.04, !- Loads Convergence Tolerance Value" << "\r\n"
		<< "    0.4, !- Temperature Convergence Tolerance Value "
		<< "{deltaC}" << "\r\n"
		<< "    FullInteriorAndExterior, !- Solar Distribution"
		<< "\r\n"
		<< "    25; !- Maximum Number of Warmup Days"
		<< "\r\n" << "\r\n";
}

/* writes the default simulation timestep for the file */
void writetimestep(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: TIMESTEP");
	outfile << "  Timestep,4;" << "\r\n" << "\r\n";
}

/* writes the simulation controls for the .idf file */
void writesimulationcontrol(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: SIMULATIONCONTROL");
	outfile << "  SimulationControl," << "\r\n"
	        << "    No,   !- Do Zone Sizing Calculation" << "\r\n"
		<< "    No,   !- Do System Sizing Calculation" << "\r\n"
		<< "    No,   !- Do Plant Sizing Calculation" << "\r\n"
		<< "    Yes,  !- Run Simulation for Sizing Periods" << "\r\n"
		<< "    Yes;  !- Run Simulation for Weather File "
		<< "Run Periods" << "\r\n" << "\r\n";
}

/* writes the building location.  Since this program is unable to
 * deduce the location, a default value is specified (residing in
 * Chicago) */
void writelocation(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: LOCATION");
	outfile << "  Site:Location," << "\r\n"
	        << "    CHICAGO_IL_USA TMY2-94846, !- Name" << "\r\n"
		<< "    41.78,   !- Latitude  {deg}" << "\r\n"
		<< "    -87.75,  !- Longitude {deg}" << "\r\n"
		<< "    -6.0,    !- Time Zone {hr}" << "\r\n"
		<< "    190.00;  !- Elevation {m}" << "\r\n"
		<< "\r\n"
		<< "  GlobalGeometryRules," << "\r\n"
		<< "    UpperLeftCorner,    "
		<< "!- Starting Vertex Position" << "\r\n"
		<< "    Counterclockwise,   "
		<< "!- Vertex Entry Direction" << "\r\n"
		<< "    Absolute;           "
		<< "!- Coordinate System" << "\r\n" << "\r\n";
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
	outfile << "  ScheduleTypeLimits," << "\r\n" 
	        << "    Any Number; !- Name" << "\r\n" << "\r\n";
	writescheduletypelimit(outfile, "Fraction",0.0,1.0,"CONTINUOUS");
	writescheduletypelimit(outfile, "Temperature",-60,200,"CONTINUOUS");
	writescheduletypelimit(outfile, "On/Off",0,1,"DISCRETE");
	writescheduletypelimit(outfile, "Control Type",0,4,"DISCRETE");
	writescheduletypelimit(outfile, "Humidity",10,90,"CONTINUOUS");
	outfile << "  ScheduleTypeLimits," << "\r\n"
	        << "    Number; !- Name" << "\r\n" << "\r\n";

	/* write schedule for people, lights, plugloads, etc. */
	writescheduletypecompact(outfile, "OCCUPY-1");
	writescheduletypecompact(outfile, "LIGHTS-1");
	writescheduletypecompact(outfile, "EQUIP-1");

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
	outfile << "  RunPeriodControl:DaylightSavingTime," << "\r\n"
	        << "    2nd Sunday in March, !- Start Date" << "\r\n"
		<< "    1st Sunday in November; !- End Date" << "\r\n" 
		<< "\r\n";
}

void writedictionary(ofstream& outfile)
{
	writesection(outfile, 
		"ALL OBJECTS IN CLASS: OUTPUT:VARIABLEDICTIONARY");
	outfile << "  Output:VariableDictionary,IDF;" << "\r\n" << "\r\n";
}

void writefooter(ofstream& outfile)
{
	writesection(outfile, "OUTPUT VARIABLES");
	outfile << "  Output:Meter,Photovoltaic:ElectricityProduced,"
	        << "runperiod;" << "\r\n" << "\r\n";
	outfile << "  OutputControl:Table:Style," << "\r\n"
	        << "    TabAndHTML;         !- Column Separator" << "\r\n"
		<< "\r\n";
	outfile << "  Output:Table:SummaryReports," << "\r\n"
	        << "    AllSummary;         !- Report 1 Name" << "\r\n"
		<< "\r\n";
}

/* writes a full-line comment to an .idf file */
void writesection(ofstream& outfile, const string& text)
{
	outfile << " !- === " << text << " ===" << "\r\n" << "\r\n";
}

/* writes one material to an .idf file with the specified properties */
void writemat(ofstream& outfile, const string& name, 
		const string& roughness, 
		double thickness, double conductivity, double density,
		double specific_heat, double thermal_absorptance,
		double solar_absorptance, double visible_absorptance)
{
	outfile << "  Material," << "\r\n"
		<< "    " << name << ", !- Name" << "\r\n"
		<< "    " << roughness << ", !- Roughness" << "\r\n"
		<< "    " << thickness << ", !- Thickness {m}" << "\r\n"
		<< "    " << conductivity << ", !- Conductivity {W/m-K}" 
		<< "\r\n"
		<< "    " << density << ", !- Density {kg/m3}" << "\r\n"
		<< "    " << specific_heat << ", !- Specific Heat {J/kg-K}"
		<< "\r\n"
		<< "    " << thermal_absorptance 
		<< ", !- Thermal Absorptance" << "\r\n"
		<< "    " << solar_absorptance << ", !- Solar Absorptance"
		<< "\r\n"
		<< "    " << visible_absorptance 
		<< "; !- Visible Absorptance" << "\r\n" << "\r\n";
}

/* writes one material for gas (a.k.a. air) */
void writeairgap(ofstream& outfile, const string& name, 
		double thermal_resistance)
{
	outfile << "  Material:AirGap," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    " << thermal_resistance
		<< "; !- Thermal Resistance {m2-K/W}" << "\r\n" << "\r\n";
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
	outfile << "  WindowMaterial:Glazing," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    " << optical_data_type << ", !- Optical Data Type"
		<< "\r\n"
		<< "    " << dataset_name << ", !- Data Set Name" << "\r\n"
		<< "    " << thickness << ", !- Thickness {m}" << "\r\n"
		<< "    " << solar_transmittance
		<< ", !- Solar Transmittance at Normal Incidence" << "\r\n"
		<< "    " << front_solar_reflectance
		<< ", !- Front Side Solar Reflectance at Normal Incidence"
		<< "\r\n"
		<< "    " << back_solar_reflectance
		<< ", !- Back Side Solar Reflectance at Normal Incidence"
		<< "\r\n"
		<< "    " << visible_transmittance
		<< ", !- Visible Transmittance at Normal Incidence"
		<< "\r\n"
		<< "    " << front_visible_reflectance
		<< ", !- Front Side Visible Reflectance at Normal Incidence"
		<< "\r\n"
		<< "    " << back_visible_reflectance
		<< ", !- Back Side Visible Reflectance at Normal Incidence"
		<< "\r\n"
		<< "    " << infrared_transmittance
		<< ", !- Infrared Transmittance at Normal Incidence"
		<< "\r\n"
		<< "    " << front_infrared_emissivity
		<< ", !- Front Side Infrared Hemispherical Emissivity"
		<< "\r\n"
		<< "    " << back_infrared_emissivity
		<< ", !- Back Side Infrared Hemispherical Emissivity"
		<< "\r\n"
		<< "    " << conductivity << "; !- Conductivity {W/m-K}"
		<< "\r\n" << "\r\n";
}

/* writes window gas to .idf file, which describes the air gap within
 * a window. */
void writewindowgas(ofstream&outfile, const string& name, 
		const string& type, double thickness)
{
	outfile << "  WindowMaterial:Gas," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    " << type << ", !- Type" << "\r\n"
		<< "    " << thickness << "; !- Thickness {m}"
		<< "\r\n" << "\r\n";
}

/* writes one construction to file.  This represents a set of materials,
 * layered, with the outer material described first, working inwards */
void writeconstruction(ofstream& outfile, const string& name, 
		const string* layers, int num_layers)
{
	int i;

	/* write name */
	outfile << "  Construction," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n";
	
	/* write layers */
	for(i = 0; i < num_layers-1; i++)
		outfile << "    " << layers[i] << ", !- Layer " 
			<< (i+1) << "\r\n";

	/* write last layer */
	outfile << "    " << layers[num_layers-1] 
		<< "; !- Inner Layer" << "\r\n" << "\r\n";
}

/* write a schedule type limit, which specifies upper and lower bounds
 * for some value */
void writescheduletypelimit(ofstream& outfile, const string& name,
		double lower, double upper, const string& type)
{
	outfile << "  ScheduleTypeLimits,"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    " << lower << ", !- Lower Limit Value {A3}" << "\r\n"
		<< "    " << upper << ", !- Upper Limit Value {A3}" << "\r\n"
		<< "    " << type << "; !- Numeric Type" << "\r\n" << "\r\n";
}

void writescheduletypecompact(std::ofstream& outfile, 
		const std::string& name)
{
	outfile << "  Schedule:Compact," << "\r\n"
	        << "    " << name << ",      !- Name" << "\r\n"
	        << "    Fraction,          !- Schedule Type Limits Name"
			<< "\r\n"
	        << "    Through: 12/31,    !- Field 1" << "\r\n"
	        << "    For: "
			<< "WeekDays SummerDesignDay "
			<< "CustomDay1 CustomDay2, !- Field 2" << "\r\n"
	        << "    Until: 8:00,0.05,  !- Field 3" << "\r\n"
	        << "    Until: 11:00,1.00, !- Field 5" << "\r\n"
	        << "    Until: 12:00,0.80, !- Field 7" << "\r\n"
	        << "    Until: 13:00,0.40, !- Field 9" << "\r\n"
	        << "    Until: 14:00,0.80, !- Field 11" << "\r\n"
	        << "    Until: 18:00,1.00, !- Field 13" << "\r\n"
	        << "    Until: 19:00,0.50, !- Field 15" << "\r\n"
	        << "    Until: 24:00,0.0,  !- Field 17" << "\r\n"
	        << "    For: " 
			<< "Weekends WinterDesignDay Holiday, "
			<< "!- Field 25" << "\r\n"
	        << "    Until: 24:00,0.05; !- Field 26" << "\r\n" << "\r\n";
}

/* the building model can specify special days for scheduling purposes.
 * The following function will write one of them. */
void writeholiday(ofstream& outfile, const string& name, 
		const string& start, int duration, const string& type)
{
	outfile << "  RunPeriodControl:SpecialDays," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    " << start << ", !- Start Date" << "\r\n"
		<< "    " << duration << ", !- Duration {days}" << "\r\n"
		<< "    " << type << "; !- Special Day Type" << "\r\n" 
		<< "\r\n";
}

/* specifies thermostat levels */
void writethermostat(ofstream& outfile, const string& name, 
		double heating_set, double cooling_set)
{
	writesection(outfile, "HVACTemplate:Thermostat");
	outfile << "  HVACTemplate:Thermostat," << "\r\n"
	        << "    " << name << ", !- Name" << "\r\n"
		<< "    , !- Heating Setpoint Schedule Name" << "\r\n"
		<< "    " << heating_set 
		<< ", !- Constant Heating Setpoint {C}" << "\r\n"
		<< "    , !- Cooling Setpoint Schedule Name" << "\r\n"
		<< "    " << cooling_set 
		<< "; !- Constant Cooling Setpoint {C}" << "\r\n" << "\r\n";
}
