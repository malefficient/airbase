#include "fingerlib-datatypes.h"


void fill_in_default_filenames(char *path_to_db, struct db_filename_struct &global_cfg)
{


	strncpy(global_cfg.path_to_db, path_to_db, sizeof(global_cfg.path_to_db));

	char *back = global_cfg.path_to_db + strlen(global_cfg.path_to_db) - 2;

        int cnt = 0;
        //cut off trailing slashes
        if (  *(global_cfg.path_to_db + strlen(global_cfg.path_to_db) - 1) == '/'  ||
          (   *(global_cfg.path_to_db + strlen(global_cfg.path_to_db) - 1) == '\\') )
          {
                  *(global_cfg.path_to_db + strlen(global_cfg.path_to_db) - 1) = '\0';
                }

        while ( (*back != '/') && (*back != '\\')  && cnt < strlen(global_cfg.path_to_db))
        {
                back--;
                cnt++;
        }

        strncpy(global_cfg.db_name, back + 1, sizeof(global_cfg.db_name));

	strncpy(global_cfg.pcap_id_to_macaddys_fname,global_cfg.path_to_db , sizeof(global_cfg.pcap_id_to_macaddys_fname));
        strcat(global_cfg.pcap_id_to_macaddys_fname, "/mac_addys.txt");

        strncpy(global_cfg.path_to_implementation_records, global_cfg.path_to_db, sizeof(global_cfg.path_to_pcaps));
        strcat(global_cfg.path_to_implementation_records, "/../driver_db.txt");

        strncpy(global_cfg.path_to_pcaps, global_cfg.path_to_db, sizeof(global_cfg.path_to_pcaps));
        strcat(global_cfg.path_to_pcaps, "/pcaps");

        strncpy(global_cfg.path_to_prints, global_cfg.path_to_db , sizeof(global_cfg.path_to_prints));
        strcat(global_cfg.path_to_prints, "/prints");

        strncpy(global_cfg.coefficient_fname, global_cfg.path_to_db , sizeof(global_cfg.coefficient_fname));
        sprintf(global_cfg.coefficient_fname, "%s/%s.coeffic", global_cfg.path_to_db, global_cfg.db_name);

}
void print_db_filename_struct(struct db_filename_struct &global_cfg)
{
		printf("passed in path: %s\n", global_cfg.path_to_db);
        printf("path to prints: %s\n", global_cfg.path_to_prints);
        printf("path to driver info: %s\n", global_cfg.path_to_implementation_records);
        printf("path to pcaps: %s\n", global_cfg.path_to_pcaps);
        printf("path to mac-addys: %s\n", global_cfg.pcap_id_to_macaddys_fname);
        printf("coefficient-fname: %s\n", global_cfg.coefficient_fname);
}
//For now this reads out our CSV's, later it may deal  with a real DB.
DriverId2ImplementationRecordType ReadInDriverInfo(char *fname, bool verbose)
{
	DriverId2ImplementationRecordType M;
	char line[2048];
	int read;
 	FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
		printf("Error reading driver database in from %s\n", fname);
		exit(0);
		
    }                      
	ImplementationRecord R;	
 	fgets(line,sizeof(line), fp); 
	while (feof(fp) == 0)
    {
        read = strlen(line);
        if (line[0] == '#')
        {
            fgets(line,sizeof(line), fp);
            continue;
        }

		if (R.ParseCSV(line) == false)
		{
			printf("Skipping line: %s", line);
			getchar();
            fgets(line,sizeof(line), fp);
            continue;
			
		}	
		M[R.id] = R;
        fgets(line,sizeof(line), fp);
    } 	

	fclose(fp);
	return M;


}

FILE * OpenReadPrintFile(char *fname)
{
 	FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
        perror("fopen");
    }                      
	return fp;
	//If we had any file header, parse it here.
}

FILE * OpenWritePrintFile(char *fname)
{
 	FILE *fp = fopen(fname, "w+");
    if (fp == NULL)
    {
        perror("fopen");
    }                      
	//If we had any file header, write it here.
	return fp;
}

