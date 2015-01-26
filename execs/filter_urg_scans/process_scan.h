#ifndef PROCESS_SCAN_H
#define PROCESS_SCAN_H

/**
 * @file process_scan.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains processing functions for filtering
 * urg scans and exporting the results to file.
 */

#include <vector>
#include <string>
#include <fstream>
#include <timestamp/sync_xml.h>

/* the following filetypes are used by this processing */
#define TIME_SYNC_EXT     "xml"  /* file extension, time sync input */
#define URG_SCAN_EXT      "dat"  /* file extension, urg scan input */
#define FILTERED_SCAN_EXT "fss"  /* file extension, output filtered scan */

/**
 * Given a single scan file, will import, filter, and export scans
 *
 * Will filter the provided scan file and export to the specified filetype.
 *
 * @param timesync     Time-synchronization parameters for this scanner
 * @param infilename   The input scan file
 * @param outfilename  The output file
 *
 * @return    Returns zero on success, non-zero on failure.
 */
int process_scan(SyncXml& timesync, 
                 const std::string& infilename,
                 const std::string& outfilename);


#endif
