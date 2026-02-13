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

void CreateJSON(const char *filename, int iCO2, int iTemp, int iHumidity)
{
FILE *f;
char szTemp[256], szOut[1024];
time_t tt;

	f = fopen(filename, "w");
	if (f) {
		tt = time(NULL);  // get the current system time
		strcpy(szOut, "{\n  \"data\": [\n    {\n");
// CO2 value
		strcat(szOut, "      \"make\": \"Sensirion\",\n      \"model\": \"SCD41\",\n");
		strcat(szOut, "      \"kind\": \"co2\",\n");
		sprintf(szTemp, "      \"value\": %d,\n", iCO2);
		strcat(szOut, szTemp);
		strcat(szOut, "      \"unit\": \"ppm\",\n");
		sprintf(szTemp, "      \"updated_at\": %lu\n    },\n    {\n", (unsigned long)tt);
		strcat(szOut, szTemp);
// Temperature value
                strcat(szOut, "      \"make\": \"Sensirion\",\n      \"model\": \"SCD41\",\n");
                strcat(szOut, "      \"kind\": \"temperature\",\n");
                sprintf(szTemp, "      \"value\": %.2f,\n", ((float)iTemp)/10.0f);
                strcat(szOut, szTemp);
                strcat(szOut, "      \"unit\": \"celcius\",\n");
                sprintf(szTemp, "      \"updated_at\": %lu\n    },\n    {\n", (unsigned long)tt);
                strcat(szOut, szTemp);
// Humidity value
                strcat(szOut, "      \"make\": \"Sensirion\",\n      \"model\": \"SCD41\",\n");
                strcat(szOut, "      \"kind\": \"humidity\",\n");
                sprintf(szTemp, "      \"value\": %d,\n", iHumidity);
                strcat(szOut, szTemp);
                strcat(szOut, "      \"unit\": \"percent\",\n");
                sprintf(szTemp, "      \"updated_at\": %lu\n    }\n  ]\n}\n", (unsigned long)tt);
                strcat(szOut, szTemp);
		fwrite(szOut, 1, strlen(szOut), f);
		fflush(f);
		fclose(f);
	}
} /* CreateJSON() */

int main(int argc, char *argv[])
{
int i, iBus;
DIR *pDir;
struct dirent *pDE;
uint32_t u32Buses = 0; // available I2C bus numbers (0-31)

        
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
		co2.getSample();
                CreateJSON(argv[1], co2.co2(), co2.temperature(), co2.humidity());
		usleep(5000000); // one sample every 5 seconds
	}
return 0;
} /* main() */
