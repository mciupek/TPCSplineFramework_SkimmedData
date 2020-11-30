# TPCSplineFramework_SkimmedData
## Creation of Spline Input (THnSparse and TTree)

The loop over the flattrees is performed in the AliSkimmedDataAnalysisMaker.cpp and AliSkimmedDataAnalysisMaker.h macro. 
- The ThnSparse is created in the following functions of the macro:
  - AliSkimmedDataAnalysis::read(TString file_V0, Bool_t enablepileUpCut)
  - AliSkimmedDataAnalysis::Read_tracktree(TSring filename_track,Bool_t enablePileUpCut).
- The TTree containing protons are created in the following functions: 
  - AliSkimmedDataAnalysis::Filltreeformap_V0(TString file_V0, Bool_t enablepileUpCut) 
  - AliSkimmedDataAnalysis::Filltreeformap_track(TString filename_track,Bool_t enablePileUpCut);

The Trees are defined in a different class called, this need to be changed if you change the content of the flattrees: 
- TrackFlatAna.C 
- TrackFlatAna.h
- V0FlatAna.C
- V0FlatAna.h

To create the above functions again, when changing the flattree content you can use the TTree::MakeClass(const char* classname = 0, Option_t* option = "") function of the TTree class.

To initiate the loop you need to call the runV0.C macro. Make sure your specify the list of files for the flattree containing the V0 and primary tracks. In the runV0.C macro the AliskimmedDataAnaylsisMaker.so file is loaded.

To create the .so file, enter the singularity container and perform the command "make AliSkimmedDataAnalysisMaker.so". Remember that you need to call this every time you change something in the AliSkimmedDataAnalysisMaker.cpp macro.

