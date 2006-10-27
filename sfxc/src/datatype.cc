  while (1) {
    //look for MK4 type data
    // get next line
    if (fgets(line,lineLength,ctrlP) == (char *) NULL) break;
    //split line contents in key and value
    if (sscanf(line,"%s %s\n",key,val) == 2) {
      //parse the MK4 type data
      retval = retval + getLongVal(key,val,"TBR",tbr);
      retval = retval + getLongVal(key,val,"FO",fo);
      retval = retval + getLongVal(key,val,"BPS",bps);
      retval = retval + getLongVal(key,val,"NHS",nhs);
      retval = retval + getLongVal(key,val,"TPHS",tphs);
      retval = retval + getLongVal(key,val,"NCH",nch);
      retval = retval + getLongVal(key,val,"BOFF",boff);
      retval = retval + getLongVal(key,val,"SYNHS1",synhs1);
      retval = retval + getLongVal(key,val,"SYNHS2",synhs2);
      retval = retval + getLongVal(key,val,"MOD",mod);

      if (strcmp(key,"MK4FILE") == 0) strcpy(mk4file,val);
      if (strcmp(key,"HDRMAP") == 0)  strcpy(hdrmap,val);
      if (strcmp(key,"MODPAT") == 0)  strcpy(modpat,val);

      if( strcmp(key,"SIGN") == 0 ) {
        strcpy(copy,line);
        s = strtok(copy," ");
        s = strtok((char*)0," ");
        retval = retval + str2int(s,vall);//convert to long
        hs = vall;

        i=0;
        s = strtok((char*)0," ");
        while (s != 0 && i< fo) {
          retval = retval + str2int(s,vall);//convert to long
          signBS[i] = vall;
          s = strtok((char*)0," ");
          i++;
        }
      }

      if(strcmp(key,"MAGN") == 0 && magnCount < nch) {
        strcpy(copy,line);
        s = strtok(copy," ");
        s = strtok((char*)0," ");
        retval = retval + str2int(s,vall);//convert to long
        hm = vall;

        i=0;
        s = strtok((char*)0," ");
        while (s != 0 && i< fo) {
          retval = retval + str2int(s,vall);//convert to long
          magnBS[i] = vall;
          s = strtok((char*)0," ");
          i++;
        }
      }
    }
  }//end while loop MK4 type data
