void extractTree(const char* input, const char* splines, const char* DataOrMc, const char* period, const char* pass, const char* collsys,const char* inputCosmics){
	//const char* input="/media/nschmidt/Daten2/HIWI_SPLINES/GSIOutput/2015/LHC15o/pass1/AOD/V0Selected.root";
	//const char* input="/media/nschmidt/Daten2/HIWI_SPLINES/GridOutput/pPb/LHC13c_pass4/AOD/V0Selected.root";
	gROOT->LoadMacro("/media/nschmidt/Daten2/HIWI_SPLINES/GSIOutput/2015/LHC15f/pass2/AOD/splines/AliTPCcalibResidualPID.cxx+g");
	Int_t recoPass = 2;
	AliTPCcalibResidualPID resp;
	//TString runList="245683,246807,246808,246809,246810,246844,246845,246846,246847,246851,246855,246858,246859,246864,246865,246867,246870,246871,246928,246930,246937,246942,246945,246948,246949,246980,246982,246984,246989,246991,246994";
	//~ TString runList="225000, 225011, 225016, 225026, 225031, 225035, 225037, 225041, 225043, 225050, 225051, 225052, 225105, 225106, 225305, 225307, 225309, 225310, 225313, 225314, 225315, 225322, 225576, 225578, 225579, 225580, 225582, 225586, 225587, 225589, 225609, 225611, 225705, 225707, 225708, 225709, 225710, 225716, 225717, 225719, 225753, 225757, 225762, 225763, 225766, 225768, 226062, 226085, 226170, 226175, 226176, 226177, 226183, 226208, 226210, 226212, 226217, 226220, 226225, 226444, 226445, 226452, 226466, 226468, 226472, 226476, 226483, 226495, 226500, 226532, 226543, 226551, 226554, 226569, 226573, 226591, 226593, 226596, 226600, 226602, 226603, 226605, 226606";
	TString runList="225000, 225011, 225016, 225026, 225031, 225035, 225037, 225041, 225043, 225050, 225051, 225052, 225105, 225106, 225305, 225307, 225309, 225310, 225313, 225314, 225315, 225322, 225576, 225578, 225579";
	//TString runList="12345,12346,12347";
	Bool_t excludeRuns=kFALSE;
	THnSparseF* hist = resp.ProcessV0TreeFile(input,recoPass,runList,excludeRuns,inputCosmics);
	TFile* v0TreeFile = new TFile("TPCresidualPID.root","RECREATE"); 
	//TDirectory* subDir = v0TreeFile->mkdir("TPCresPID");
   //subDir->cd(); 
	hist->Write("fHistPidQA",TObject::kOverwrite);
	v0TreeFile->Write();
   v0TreeFile->Close();
	//2.6,14.3,-15,2.2,2.7,0.06};
	//AliTPCcalibResidualPID::ExtractResidualPID(hist,kTRUE,splines,DataOrMc,period,pass,collsys,0x0,"",AliTPCcalibResidualPID::kAleph);
	AliTPCcalibResidualPID::ExtractResidualPID(hist,kTRUE,splines,DataOrMc,period,pass,collsys,0x0,"",AliTPCcalibResidualPID::kAleph);
}
