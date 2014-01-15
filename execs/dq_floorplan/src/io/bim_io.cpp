#include "bim_io.h"
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include "../rooms/tri_rep.h"
#include "../structs/triple.h"
#include "../util/error_codes.h"

using namespace std;

/* the following helper functions are used in conjunction
 * with writeidf(), and are defined at the bottom of this file. */
int writeroom(ofstream& outfile, tri_rep_t& trirep, 
			set<triple_t>& room, int num);

void writeversion(ofstream& outfile);
void writebuilding(ofstream& outfile, const string& name);
void writetimestep(ofstream& outfile);
void writesimulationcontrol(ofstream& outfile);
void writelocation(ofstream& outfile);
void writecommonmats(ofstream& outfile);
void writecommonconstructions(ofstream& outfile);
void writedefaultschedule(ofstream& outfile);
void writedictionary(ofstream& outfile);
void writefooter(ofstream& outfile);

void writesection(ofstream& outfile, const string& text);
void writemat(ofstream& outfile, const string& name, 
		const string& roughness, 
		double thickness, double conductivity, double density,
		double specific_heat, double thermal_absorptance,
		double solar_absorptance, double visible_absorptance);
void writeairgap(ofstream& outfile, const string& name, 
		double thermal_resistance);
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
		double conductivity);
void writewindowgas(ofstream& outfile, const string& name, 
		const string& type, double thickness);
void writeconstruction(ofstream& outfile, const string& name, 
		const string* layers, int num_layers);
void writescheduletypelimit(ofstream& outfile, const string& name,
		double lower, double upper, const string& type);
void writeholiday(ofstream& outfile, const string& name, 
		const string& start, int duration, const string& type);
void writethermostat(ofstream& outfile, const string& name, 
		double heating_set, double cooling_set);

/********************* FUNCTION BODIES **************************/

/* write a .idf file for energy plus */
int writeidf(char* filename, tri_rep_t& trirep)
{
	vector<set<triple_t> > rooms;
	ofstream outfile;
	int i, n, ret;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* write basic info about the model
	 * (this doesn't change between models) */
	writeversion(outfile);
	writebuilding(outfile, string(filename));
	writetimestep(outfile);
	writesimulationcontrol(outfile);
	writelocation(outfile);
	writecommonmats(outfile);
	writecommonconstructions(outfile);
	writedefaultschedule(outfile);
	writethermostat(outfile, "Constant Setpoint Thermostat", 20, 25); 
	writedictionary(outfile);

	/* compute the geometry for each room, and write to file */
	trirep.get_rooms(rooms);
	n = rooms.size();
	for(i = 0; i < n; i++)
	{
		ret = writeroom(outfile, trirep, rooms[i], i);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* write footer information of this file */
	writefooter(outfile);

	/* clean up */
	outfile.close();
	return 0;
}

/**************** geometry helper functions ***************************/

int writeroom(ofstream& outfile, tri_rep_t& trirep, 
				set<triple_t>& room, int num)
{
	map<triple_t, tri_info_t>::iterator tit;
	map<triple_t, room_height_t>::iterator hit;
	vector<vector<int> > edge_list;
	point_t p, q;
	int ret, b, i, n;

	/* verify we have a non-trivial room */
	if(room.empty())
		return -1;

	/* compute the edges of this room, which will become walls */
	ret = tri_rep_t::compute_boundary_edges(edge_list, room);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	
	/* verify we have edges */
	n = edge_list.size(); /* how many disjoint boundaries are there? */
	if(n <= 0)
		return -3;

	/* check the number of boundaries to this room (we only want
	 * to use the outermost one, which causes us to ignore columns
	 * and islands within the room */
	b = 0; /* b denotes the boundary to use */
	for(i = 1; i < n; i++)
		if(edge_list[i].size() > edge_list[b].size())
			b = i; /* use the biggest boundary by size, which
				* ideally should be the outermost one */

	// TODO, need to associate the same wall in separate rooms with
	// each other.

	/* retrieve the height of this room */
	tit = trirep.tris.find(*(room.begin()));
	if(tit == trirep.tris.end())
		return -4;
	hit = trirep.room_heights.find(tit->second.root);
	if(hit == trirep.room_heights.end())
		return -5;

	/* create a zone for this room */
	writesection(outfile, "ROOM GEOMETRY");
	outfile << "  Zone," << endl
	        << "    Room_" << num << ", !- Name" << endl
		<< "    0.0,           !- Direction of Relative "
		<< "North {deg}" << endl
		<< "    0.0,           !- X Origin {m}" << endl
		<< "    0.0,           !- Y Origin {m}" << endl
		<< "    0.0,           !- Z Origin {m}" << endl
		<< "    ,              !- Type" << endl
		<< "    1,             !- Multiplier" << endl
		<< "    autocalculate, !- Ceiling Height {m}" << endl
		<< "    autocalculate; !- volume {m3}" << endl << endl;

	/* create a floor for this room */
	n = edge_list[b].size();
	outfile << "  BuildingSurface:Detailed," << endl
	        << "    Zn" << num << ":Flr0, !- Name" << endl
		<< "    Floor,         !- Surface Type" << endl
		<< "    Interior Floor,!- Construction Name" << endl
		<< "    Room_" << num << ", !- Zone Name" << endl
		<< "    Surface,       !- Outside Boundary Condition"
		<< endl
		<< "    Zn" << num << ":Flr0, !- Outside Boundary Condition"
		<< " Object" << endl
		<< "    NoSun,         !- Sun Exposure" << endl
		<< "    NoWind,        !- Wind Exposure" << endl
		<< "    1.0,           !- View Factor to Ground" << endl
		<< "    " << n << ",   !- Number of Vertices" << endl;

	/* define geometry of floor */
	for(i = 0; i < n; i++)
	{
		/* get point along boundary */
		p = trirep.pos(edge_list[b][i]);
	
		/* add to floor */
		outfile << "    " << p.get(0) << "," << p.get(1) << ","
		        << hit->second.min_z
			<< (i == n-1 ? ";" : ",")
			<< "   !- Vertex " << i << " {m}" << endl;
	}
	outfile << endl;
	
	/* create a ceiling for this room */
	outfile << "  BuildingSurface:Detailed," << endl
	        << "    Zn" << num << ":Ceil0, !- Name" << endl
		<< "    Ceiling,         !- Surface Type" << endl
		<< "    Interior Ceiling,!- Construction Name" << endl
		<< "    Room_" << num << ", !- Zone Name" << endl
		<< "    Outdoors,         !- Outside Boundary Condition"
		<< endl
		<< "    , !- Outside Boundary Condition"
		<< " Object" << endl
		<< "    NoSun,           !- Sun Exposure" << endl
		<< "    NoWind,          !- Wind Exposure" << endl
		<< "    0,             !- View Factor to Ground" << endl
		<< "    " << n << ",     !- Number of Vertices" << endl;

	/* define geometry of ceiling */
	for(i = n-1; i >= 0; i--)
	{
		/* get point along boundary */
		p = trirep.pos(edge_list[b][i]);
	
		/* add to ceiling */
		outfile << "    " << p.get(0) << "," << p.get(1) << ","
		        << hit->second.max_z
			<< (i == 0 ? ";" : ",")
			<< "   !- Vertex " << (n-1-i) << " {m}" << endl;
	}
	outfile << endl;

	/* export each edge as a separate wall */
	for(i = 0; i < n; i++)
	{
		/* get points for current edge along this boundary */
		p = trirep.pos(edge_list[b][i]);
		q = trirep.pos(edge_list[b][(i+1)%n]);

		/* make a wall */
		outfile << "  BuildingSurface:Detailed," << endl
		        << "    Zn" << num << ":Wall" << i << ", !- Name"
			<< endl
			<< "    Wall,          !- Surface Type" << endl
			<< "    Interior Wall, !- Construction Name" << endl
			<< "    Room_" << num << ", !- Zone name" << endl
			<< "    Surface,       !- Outside Boundary "
			<< "Condition" << endl
			<< "    Zn" << num << ":Wall" << i // TODO link
			<< ", !- Outside Boundary Condition Object" << endl
			<< "    NoSun,         !- Sun Exposure" << endl
			<< "    NoWind,        !- Wind Exposure" << endl
			<< "    ,              !- View Factor to Ground"
			<< endl
			<< "    4,             !- Number of Vertices" 
			<< endl
			<< "    " << p.get(0) << "," << p.get(1) 
			<< "," << hit->second.min_z 
			<< ", !- Vertex 1, lower right {m}" << endl
			<< "    " << p.get(0) << "," << p.get(1) 
			<< "," << hit->second.max_z 
			<< ", !- Vertex 2, upper right {m}" << endl
			<< "    " << q.get(0) << "," << q.get(1) 
			<< "," << hit->second.max_z 
			<< ", !- Vertex 3, upper left {m}" << endl
			<< "    " << q.get(0) << "," << q.get(1) 
			<< "," << hit->second.min_z 
			<< "; !- Vertex 4, lower left {m}" << endl
			<< endl;
	}
	
	/* success */
	return 0;
}

/**************** implementation of helper functions ******************/

/* writes the version of Energy Plus that is supported */
void writeversion(ofstream& outfile)
{
	writesection(outfile, "ALL OBJECTS IN CLASS: VERSION");
	outfile << "  Version,7.1;" << endl << endl;
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
