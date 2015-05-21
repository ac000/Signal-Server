/*
 * sdf.c - Functions for dealing with SDF files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"

extern int ippd;
extern int jgets;
extern int min_elevation;
extern int max_elevation;
extern int min_north;
extern int max_north;
extern int min_west;
extern int max_west;

extern char sdf_path[];

extern struct dem dem[];

extern int debug;

int LoadSDF_SDF(char *name, int winfiles)
{
	/* This function reads uncompressed ss Data Files (.sdf)
	   containing digital elevation model data into memory.
	   Elevation data, maximum and minimum elevations, and
	   quadrangle limits are stored in the first available
	   dem[] structure. */

	int x, y, data, indx, minlat, minlon, maxlat, maxlon, j;
	char found, free_page = 0, line[20], jline[20], sdf_file[255],
	    path_plus_name[255], *junk = NULL;

	FILE *fd;

	for (x = 0; name[x] != '.' && name[x] != 0 && x < 250; x++)
		sdf_file[x] = name[x];

	sdf_file[x] = 0;

	/* Parse filename for minimum latitude and longitude values */
	if (winfiles == 1) {
		sscanf(sdf_file, "%d=%d=%d=%d", &minlat, &maxlat, &minlon,
		       &maxlon);
	} else {
		sscanf(sdf_file, "%d:%d:%d:%d", &minlat, &maxlat, &minlon,
		       &maxlon);
	}

	sdf_file[x] = '.';
	sdf_file[x + 1] = 's';
	sdf_file[x + 2] = 'd';
	sdf_file[x + 3] = 'f';
	sdf_file[x + 4] = 0;

	/* Is it already in memory? */

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0; indx++) {
		if (minlat == dem[indx].min_north
		    && minlon == dem[indx].min_west
		    && maxlat == dem[indx].max_north
		    && maxlon == dem[indx].max_west)
			found = 1;
	}

	/* Is room available to load it? */

	if (found == 0) {
		for (indx = 0, free_page = 0; indx < MAXPAGES && free_page == 0;
		     indx++)
			if (dem[indx].max_north == -90)
				free_page = 1;
	}

	indx--;

	if (free_page && found == 0 && indx >= 0 && indx < MAXPAGES) {
		/* Search for SDF file in current working directory first */

		strncpy(path_plus_name, sdf_file, 255);

		fd = fopen(path_plus_name, "rb");

		if (fd == NULL) {
			/* Next, try loading SDF file from path specified
			   in $HOME/.ss_path file or by -d argument */

			strncpy(path_plus_name, sdf_path, 255);
			strncat(path_plus_name, sdf_file, 255);
			fd = fopen(path_plus_name, "rb");
		}

		if (fd != NULL) {
			if (debug == 1) {
				fprintf(stdout,
					"Loading \"%s\" into page %d...",
					path_plus_name, indx + 1);
				fflush(stdout);
			}

			if (fgets(line, 19, fd) != NULL) {
				sscanf(line, "%d", &dem[indx].max_west);
			}

			if (fgets(line, 19, fd) != NULL) {
				sscanf(line, "%d", &dem[indx].min_north);
			}

			if (fgets(line, 19, fd) != NULL) {
				sscanf(line, "%d", &dem[indx].min_west);
			}

			if (fgets(line, 19, fd) != NULL) {
				sscanf(line, "%d", &dem[indx].max_north);
			}
			/*
			   Here X lines of DEM will be read until IPPD is reached.
			   Each .sdf tile contains 1200x1200 = 1.44M 'points'
			   Each point is sampled for 1200 resolution!
			 */
			for (x = 0; x < ippd; x++) {
				for (y = 0; y < ippd; y++) {

					for (j = 0; j < jgets; j++) {
						junk = fgets(jline, 19, fd);
					}

					if (fgets(line, 19, fd) != NULL) {
						data = atoi(line);
					}

					dem[indx].data[x][y] = data;
					dem[indx].signal[x][y] = 0;
					dem[indx].mask[x][y] = 0;

					if (data > dem[indx].max_el)
						dem[indx].max_el = data;

					if (data < dem[indx].min_el)
						dem[indx].min_el = data;

				}

				if (ippd == 600) {
					for (j = 0; j < IPPD; j++) {
						junk = fgets(jline, 19, fd);
					}
				}
				if (ippd == 300) {
					for (j = 0; j < IPPD; j++) {
						junk = fgets(jline, 19, fd);
						junk = fgets(jline, 19, fd);
						junk = fgets(jline, 19, fd);

					}
				}
			}

			fclose(fd);

			if (dem[indx].min_el < min_elevation)
				min_elevation = dem[indx].min_el;

			if (dem[indx].max_el > max_elevation)
				max_elevation = dem[indx].max_el;

			if (max_north == -90)
				max_north = dem[indx].max_north;

			else if (dem[indx].max_north > max_north)
				max_north = dem[indx].max_north;

			if (min_north == 90)
				min_north = dem[indx].min_north;

			else if (dem[indx].min_north < min_north)
				min_north = dem[indx].min_north;

			if (max_west == -1)
				max_west = dem[indx].max_west;

			else {
				if (abs(dem[indx].max_west - max_west) < 180) {
					if (dem[indx].max_west > max_west)
						max_west = dem[indx].max_west;
				}

				else {
					if (dem[indx].max_west < max_west)
						max_west = dem[indx].max_west;
				}
			}

			if (min_west == 360)
				min_west = dem[indx].min_west;

			else {
				if (fabs(dem[indx].min_west - min_west) < 180.0) {
					if (dem[indx].min_west < min_west)
						min_west = dem[indx].min_west;
				}

				else {
					if (dem[indx].min_west > min_west)
						min_west = dem[indx].min_west;
				}
			}

			return 1;
		}

		else
			return -1;
	}

	else
		return 0;
}

char LoadSDF(char *name, int winfiles)
{
	/* This function loads the requested SDF file from the filesystem.
	   It first tries to invoke the LoadSDF_SDF() function to load an
	   uncompressed SDF file (since uncompressed files load slightly
	   faster).  If that attempt fails, then it tries to load a
	   compressed SDF file by invoking the LoadSDF_BZ() function.
	   If that fails, then we can assume that no elevation data
	   exists for the region requested, and that the region
	   requested must be entirely over water. */

	int x, y, indx, minlat, minlon, maxlat, maxlon;
	char found, free_page = 0;
	int return_value = -1;

	return_value = LoadSDF_SDF(name, winfiles);

	/* If neither format can be found, then assume the area is water. */

	if (return_value == 0 || return_value == -1) {

		if (winfiles == 1) {
			sscanf(name, "%d=%d=%d=%d", &minlat, &maxlat, &minlon,
			       &maxlon);
		} else {
			sscanf(name, "%d:%d:%d:%d", &minlat, &maxlat, &minlon,
			       &maxlon);
		}
		/* Is it already in memory? */

		for (indx = 0, found = 0; indx < MAXPAGES && found == 0; indx++) {
			if (minlat == dem[indx].min_north
			    && minlon == dem[indx].min_west
			    && maxlat == dem[indx].max_north
			    && maxlon == dem[indx].max_west)
				found = 1;
		}

		/* Is room available to load it? */

		if (found == 0) {
			for (indx = 0, free_page = 0;
			     indx < MAXPAGES && free_page == 0; indx++)
				if (dem[indx].max_north == -90)
					free_page = 1;
		}

		indx--;

		if (free_page && found == 0 && indx >= 0 && indx < MAXPAGES) {
			if (debug == 1) {
				fprintf(stdout,
					"Region  \"%s\" assumed as sea-level into page %d...",
					name, indx + 1);
				fflush(stdout);
			}

			dem[indx].max_west = maxlon;
			dem[indx].min_north = minlat;
			dem[indx].min_west = minlon;
			dem[indx].max_north = maxlat;

			/* Fill DEM with sea-level topography */

			for (x = 0; x < ippd; x++)
				for (y = 0; y < ippd; y++) {
					dem[indx].data[x][y] = 0;
					dem[indx].signal[x][y] = 0;
					dem[indx].mask[x][y] = 0;

					if (dem[indx].min_el > 0)
						dem[indx].min_el = 0;
				}

			if (dem[indx].min_el < min_elevation)
				min_elevation = dem[indx].min_el;

			if (dem[indx].max_el > max_elevation)
				max_elevation = dem[indx].max_el;

			if (max_north == -90)
				max_north = dem[indx].max_north;

			else if (dem[indx].max_north > max_north)
				max_north = dem[indx].max_north;

			if (min_north == 90)
				min_north = dem[indx].min_north;

			else if (dem[indx].min_north < min_north)
				min_north = dem[indx].min_north;

			if (max_west == -1)
				max_west = dem[indx].max_west;

			else {
				if (abs(dem[indx].max_west - max_west) < 180) {
					if (dem[indx].max_west > max_west)
						max_west = dem[indx].max_west;
				}

				else {
					if (dem[indx].max_west < max_west)
						max_west = dem[indx].max_west;
				}
			}

			if (min_west == 360)
				min_west = dem[indx].min_west;

			else {
				if (abs(dem[indx].min_west - min_west) < 180) {
					if (dem[indx].min_west < min_west)
						min_west = dem[indx].min_west;
				}

				else {
					if (dem[indx].min_west > min_west)
						min_west = dem[indx].min_west;
				}
			}

			return_value = 1;
		}
	}

	return return_value;
}

