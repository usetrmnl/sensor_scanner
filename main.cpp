//
// Write SCD4x sensor values into a JSON file
// written by Larry Bank Feb 13, 2026
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <bb_scd41.h>
#include <time.h>

SCD41 co2;

// Structure holding all of the info for a JSON sensor record
typedef struct tagjsonrec
{
const char *szMake;
const char *szModel;
const char *szKind;
const char *szValue;
const char *szUnit;
} JSONREC;

// Enumerated value indicating the position of the record in the JSON
// We need this to know where to start/end the nested curly braces
enum {
    REC_INVALID = 0,
    REC_FIRST,
    REC_ANY,
    REC_LAST
};

//
// Add one item to the JSON file
// f - an open file handle
// pRec - pointer to a JSONREC structure
// iRecOrder - indicates if this is the first, middle or last value
//
void AddJSONItem(FILE *f, JSONREC *pRec, int iRecOrder)
{
time_t tt;
char szOut[1024], szTemp[256];

	tt = time(NULL);  // get the current system time
	if (iRecOrder == REC_FIRST) {
		strcpy(szOut, "{\n  \"data\": [\n    {\n");
        } else {
		strcpy(szOut, "    {\n");
	}
	sprintf(szTemp, "      \"make\": \"%s\",\n      \"model\": \"%s\",\n", pRec->szMake, pRec->szModel);
	strcat(szOut, szTemp);
	sprintf(szTemp, "      \"kind\": \"%s\",\n      \"value\": %s,\n", pRec->szKind, pRec->szValue);
	strcat(szOut, szTemp);
	sprintf(szTemp, "      \"unit\": \"%s\",\n", pRec->szUnit);
	strcat(szOut, szTemp);
	sprintf(szTemp, "      \"updated_at\": %lu\n    }", (unsigned long)tt);
	strcat(szOut, szTemp);
	if (iRecOrder == REC_LAST) {
		strcat(szOut, "\n  ]\n}\n");
	} else {
		strcat(szOut, ",\n");
	}
	fwrite(szOut, 1, strlen(szOut), f);
} /* AddJSONItem() */

int main(int argc, char *argv[])
{
int i, iBus;
DIR *pDir;
struct dirent *pDE;
uint32_t u32Buses = 0; // available I2C bus numbers (0-31)
JSONREC rec;
        
        printf("sensor2json example\n");
        printf("Finds which I2C bus has the supported sensor, then\ninitializes, configures, and loops updating the json with the latest sensor values.\n");
        if (argc != 2) {
            printf("usage: sensor2json <json file>\n");
            return -1;
        }
	// I2C buses in Linux are defined as a file in the /dev directory
        pDir = opendir("/dev");
	if (!pDir) {
		printf("Error searching /dev directory; aborting.\n");
		return -1;
	}
	// Search all names in the /dev directory for those starting with i2c-
        while ((pDE = readdir(pDir)) != NULL) {
		if (memcmp(pDE->d_name, "i2c-", 4) == 0) { // found one!
                    iBus = atoi(&pDE->d_name[4]);
		    u32Buses |= (1 << iBus); // collect the bus numbers
		}
	}
	closedir(pDir);
	if (u32Buses == 0) { // Something went wrong; no I2C buses
	    printf("No I2C buses found!\n");
	    printf("Check your system configuration (e.g. raspi-config)\n");
	    printf("to ensure that I2C is enabled.\n");
	    return -1;
	}
	// Search each I2C bus for a supported proximited sensor
        for (iBus=0; iBus<32; iBus++) {
	    if (u32Buses & (1<<iBus)) { // a bus that we found in /dev
                i = co2.init(iBus); // scan for supported sensors
                if (i == SCD41_SUCCESS) {
                    printf("Found a device on i2c-%d!\n", iBus);
		    break;
		}
	    }
        } // for each possible bus
	if (iBus == 32) { // scanned all buses and didn't find anything
            printf("No supported device found\n");
	    printf("Your system may require sudo to access I2C.\n");
            return -1; // problem - quit
	}
        printf("SCD41 detected and initialized\n");
	co2.start(); // start the sensor with the default options
	while (1) {
            char szValue[32];
            FILE *f;
            co2.getSample();
            f = fopen(argv[1], "w+b");
            if (f) { // open succeeded
                // The constant info for each JSON record
                rec.szMake = "Sensirion";
                rec.szModel = "SCD41";
                rec.szValue = szValue;
		// Add CO2 value
                rec.szKind = "co2";
                sprintf(szValue, "%d", co2.co2());
                rec.szUnit = "parts_per_million";
                AddJSONItem(f, &rec, REC_FIRST);
		// Add temperature value
                rec.szKind = "temperature";
                sprintf(szValue, "%.1f", (float)co2.temperature() / 10.0f);
                rec.szUnit = "celsius";
                AddJSONItem(f, &rec, REC_ANY);
		// Add humidity value
                rec.szKind = "humidity";
                sprintf(szValue, "%d", co2.humidity());
                rec.szUnit = "percent";
                AddJSONItem(f, &rec, REC_LAST);
		// write any remaining data and close the file
                fflush(f);
                fclose(f);
		usleep(5000000); // one sample every 5 seconds
            } // if file opened
	} // while (1)
return 0;
} /* main() */
