

void runV0(TString filelist="/lustre/nyx/alice/users/mciupek/TPCSpline/Tree_sigmaPara/LHC18q_pass3/data/2018/LHC18q/295854/V0_LHC18q_295854.list",
           TString filelist_track="/lustre/nyx/alice/users/mciupek/TPCSpline/Tree_sigmaPara/LHC16s_pass2/data/2016/LHC16s/266885/pass2_CENT_woSDD/track_LHC16s_FAST_266885.list", 
          ) {

  gSystem->Load("AliSkimmedDataAnalysisMaker.so");
  //gInterpreter->Load("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/code_SplineInput/BBFitAnalysis/AliSkimmedDataAnalysisMaker.so");

  //  R__LOAD_LIBRARY(HelloWorld.so);
  // V0FlatAna* V0ana = new V0FlatAna();
  // TFile* inputFile = new TFile("V0tree.root","READ");
  // TTree *tree_v0=  (TTree *) inputFile->Get("V0Flat");

  // V0ana->Init(tree_v0);
  // V0ana->Loop();
  AliSkimmedDataAnalysisMaker *ana = new AliSkimmedDataAnalysisMaker();
  //  ana->read();
  //  ana->WriteHistogram();


  // StEvtGen_Ana * EvtGen_QA=new StEvtGen_Ana(outfile);
  ana->bookHistogram();
  cout<< " booked the histogram"<<endl;
  char filename[1000];
  ifstream fstream(filelist.Data());
  int ifile = 0;
  while (fstream >> filename)
   {
      ++ifile;
      cout <<"sngl_file: "<<ifile<<" : "<<filename<<endl;
      ana->TreeV0_BBFitAnalysis(filename);
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
      ana->TreePrimary_BBFitAnalysis(filename_track);
    }



 ana->WriteHistogram();

  //  ana->execute();
}

