#include "coefficient-table.h"


Coefficient_Table::Coefficient_Table()
{
	alg_num = 0;
	flag_coefficient = 0;
	durations_match_coefficient = 0;
	durations_type_subtype_match_coefficient = 0;

}
bool Coefficient_Table::AppendToFile(char *fname, char*header)
{
	FILE *fp;
	fp = fopen(fname, "w+"); //XXX: do i really mean w+? sholuld i prompt user if i overwrite?
	if (fp == NULL)
	{
		perror("Coefficient_Table::AppendToFile:fopen");
		exit(0);
	}
	
	bool ret = AppendToFp(fp, header);
	fclose(fp);
}
bool Coefficient_Table::ReadFromFile(char *fname)
{
	FILE *fp;
	fp = fopen(fname, "r"); 
	if (fp == NULL)
	{
		perror("Coefficient_Table::ReadFromFile:fopen");
		exit(0);
	}
	bool ret = ReadFromFp(fp);
	fclose(fp);
}

bool Coefficient_Table::AppendToFp(FILE *FP, char*header)
{
	if (header != NULL)
		if (header[0] != '#')
			fputc('#', FP); //if tehy caller forgets a #, we just add it for them..

	char *c;
	c = header;
	
	if (header != NULL)
	{
		for(int i =0; i < strlen(header); i++)
		{
			putc(*c, FP);
			if(*c =='\n')
			   putc('#', FP);
	
			c++;
		}
		if (header[strlen(header)] != '\n')
			putc('\n', FP);

	}

	OutputText(FP); //for now we use the same representation on disk as on screen. this may change.
}
bool Coefficient_Table::ReadFromFp(FILE *FP)
{
	int num_lines_read = 0;
	int num_lines_to_read = 6;
	char line[256];
	char *front;
	u_int32_t *curr_peice_of_data;
	memset(line,0,sizeof(line));

	while (num_lines_read < num_lines_to_read)
	{
		fgets(line, 256, FP);
		//printf("%d--%s", num_lines_read, line);
		if (line[0] =='#')
		{
		//	printf("skipping comment: %s\n", line);
			continue; //skip comments.
		}
		num_lines_read++;
		if (num_lines_read == 1)
			curr_peice_of_data = &alg_num;
		else if (num_lines_read == 2)
			curr_peice_of_data = &flag_coefficient;
		else if (num_lines_read == 3)
			curr_peice_of_data = &durations_match_coefficient;
		else if (num_lines_read == 4)
			curr_peice_of_data = &durations_not_match_coefficient;
		else if (num_lines_read == 5)
			curr_peice_of_data = &durations_type_subtype_match_coefficient;
		else if (num_lines_read == 6)
			curr_peice_of_data = &durations_type_subtype_not_match_coefficient;
		else
		{
			printf("Coefficient_Table::ReadFromFp:Sanity check failed. has the file format changed?");
			exit(0);
		}
		
		front = line;
		while (*front != '\t' && *front != ',' && *front != ' ' && *front != '\n')//csv,tab,space, all ok delims.A
                        front++;
		//printf("Stopping at %s", front);
                if (*front == '\n' || *front == '\0')
                {
                        printf("Coefficient_Table::ReadFromFp: Error parsing line %s\n", line);
                        exit(0);
                }
                front++; //start pointing at mac
                //*(front + strlen(front) - 1) = '\0'; //nip off that newline
		int this_lines_data = atoi(front);
		*curr_peice_of_data = this_lines_data;
		//printf("just read-%d\n", this_lines_data);
		
	}
	
	return true;

}

void Coefficient_Table::OutputText(FILE *fp)
{
	fprintf(fp, "%s", toString().c_str());
}
string Coefficient_Table::toString()
{
	char buff[256];
	string ret;
	sprintf(buff,"matchiness-alg-num: %d\n", alg_num);
	ret += buff;
	sprintf(buff,"flag-coefficient:\t\t  %d\n", flag_coefficient);
	ret += buff;
	sprintf(buff,"durations_match_coefficient:\t  %d\n", durations_match_coefficient);
	ret += buff;
	sprintf(buff,"durations_not_match_coefficient:  %d\n", durations_not_match_coefficient);
	ret += buff;
	sprintf(buff,"durations_type_subtype_match:\t  %d\n", durations_type_subtype_match_coefficient);
	ret += buff;
	sprintf(buff,"durations_type_subtype_not_match: %d\n", durations_type_subtype_not_match_coefficient);
	ret += buff;
	return ret;

}


string Computation_Parameters::toString()
{
	string ret;
	char buff[256];

	sprintf(buff, "%s:%d\n", "max_algnum_to_try", max_algnum_to_try);
	ret += buff;

 	sprintf(buff, "%s:%d\n","flag_coefficient_bottom", flag_coefficient_bottom);
	ret += buff;
        sprintf(buff, "%s:%d\n","flag_coefficient_top", flag_coefficient_top);
	ret += buff;
        sprintf(buff, "%s:%d\n","flag_coefficient_delta", flag_coefficient_delta);
	ret += buff;

        sprintf(buff, "%s:%d\n","durations_match_bottom", durations_match_bottom);
	ret += buff;
        sprintf(buff, "%s:%d\n","durations_match_top", durations_match_top);
	ret += buff;
        sprintf(buff, "%s:%d\n","durations_match_delta", durations_match_delta);
	ret += buff;

        sprintf(buff, "%s:%d\n","durations_not_match_bottom", durations_not_match_bottom);
	ret += buff;
        sprintf(buff, "%s:%d\n","durations_not_match_top", durations_not_match_top);
	ret += buff;
        sprintf(buff, "%s:%d\n","durations_not_match_delta", durations_not_match_delta);
	ret += buff;

        sprintf(buff, "%s:%d\n","type_sub_match_bottom", type_sub_match_bottom);
	ret += buff;
        sprintf(buff, "%s:%d\n","type_sub_match_top", type_sub_match_top);
	ret += buff;
        sprintf(buff, "%s:%d\n","type_sub_match_delta", type_sub_match_delta);
	ret += buff;

        sprintf(buff, "%s:%d\n","type_sub_not_match_bottom", type_sub_not_match_bottom);
	ret += buff;
        sprintf(buff, "%s:%d\n","type_sub_not_match_top",	type_sub_not_match_top);
	ret += buff;
        sprintf(buff, "%s:%d\n","type_sub_not_match_delta", type_sub_not_match_delta);
	ret += buff;

	return ret;

}

void generate_coefficient_tables(list<Coefficient_Table> &L, Computation_Parameters P)
{ 
	Coefficient_Table C;
	for (int alg_num = 1; alg_num <= P.max_algnum_to_try; alg_num++) //XXX!!! make 4
        for (int flag_c = P.flag_coefficient_bottom; flag_c <= P.flag_coefficient_top; flag_c += P.flag_coefficient_delta)
        for(int duration_c = P.durations_match_bottom; duration_c <= P.durations_match_top; duration_c += P.durations_match_delta)
        for(int duration_not_c = P.durations_not_match_bottom; duration_not_c <= P.durations_not_match_top; duration_not_c += P.durations_not_match_delta)
        for(int type_sub_match_c = P.type_sub_match_bottom; type_sub_match_c <= P.type_sub_match_top; type_sub_match_c += P.type_sub_match_delta)
        for(int type_sub_not_match_c = P.type_sub_not_match_bottom; type_sub_not_match_c <= P.type_sub_not_match_top; type_sub_not_match_c += P.type_sub_not_match_delta)

       {
                C.alg_num = alg_num;
                C.flag_coefficient = flag_c;
                C.durations_match_coefficient = duration_c;
                C.durations_not_match_coefficient = duration_not_c;
                C.durations_type_subtype_match_coefficient = type_sub_match_c;
                C.durations_type_subtype_not_match_coefficient = type_sub_not_match_c;
		L.push_back(C);
	}

}

