void runV0(TString filelist="list/V0_LHC15o.list",
           TString filelist_track = "list/track_LHC15o.list", 
           Bool_t enablePileUpCut=kFALSE,
           Bool_t isPbPb = kTRUE) {
  
  gSystem->Load("/lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/code_SplineInput/AliSkimmedDataAnalysisMaker.so");

  //  R__LOAD_LIBRARY(HelloWorld.so);
  // V0FlatAna* V0ana = new V0FlatAna();
  // TFile* inputFile = new TFile("V0tree.root","READ");
  // TTree *tree_v0=  (TTree *) inputFile->Get("V0Flat");

  // V0ana->Init(tree_v0);
  // V0ana->Loop();
  AliSkimmedDataAnalysisMaker *ana=new AliSkimmedDataAnalysisMaker("TPCresidualPID.root","TPCPIDEtaTree.root");
  //  ana->read();
  //  ana->WriteHistogram();


  // StEvtGen_Ana * EvtGen_QA=new StEvtGen_Ana(outfile);
  ana->bookHistogram(isPbPb);
  cout<< " booked the histogram"<<endl;
  char filename[1000];
  ifstream fstream(filelist.Data());
  int ifile = 0;
  while (fstream >> filename)
   {
      ++ifile;
      cout <<"sngl_file: "<<ifile<<" : "<<filename<<endl;
         ana->read(filename,enablePileUpCut);
		     //ana->Filltreeformap_V0(filename,enablePileUpCut);
    }
  cout<< "this is for QA"<<endl;
  
//  ana->WriteHistogram();

// run Track tree

  char filename_track[1000];
  ifstream fstream_track(filelist_track.Data());
  int ifile_track = 0;
  while (fstream_track >> filename_track)
    {
      ++ifile_track;
      cout <<"sngl_file: "<<ifile_track<<" : "<<filename_track<<endl;
         ana->Read_tracktree(filename_track,enablePileUpCut);
	       //ana->Filltreeformap_track(filename_track,enablePileUpCut);
    }



 ana->WriteHistogram();

  //  ana->execute();
}

