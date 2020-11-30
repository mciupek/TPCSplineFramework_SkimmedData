## TPC Spline Offline Framework

In this folder one can find the code to create the splines for the TPC dE/dx parametrization. Here parts of the code should be explained coarsely.

### 0PERIOD_PASS_splineCreationv3.sh

This is the bash script which is called to start the chain of the spline creation. Make sure the information in the first lines are matching the data set your want to create the spline for. In the lines starting at line 50 you can change the binning of some of the variables if you are lacking statistics. 

To start the spline creation make sure the bash script is in the same folder than the ThNSparse and TTree file. The bash script is called via "bash 0PERIOD_PASS_splineCreationv3.sh"

### AliTPCcalibResidualPID2.cxx

In this file the Bethe-Bloch fit together with the low momentum corrections are performed. In addition QA plots for the dEdx vs p fits are created, for the low momentum correction and for the Bethe-Bloch fit are created.

In the function AliTPCcalibResidualPID2::GetResiduals() the dEdx vs p plots are created for all the particle species (electron, pion, kaon, proton, deuteron) coming from primary vertex or V0. 

At the beginning of the function one can define some lose TPCnsigma cuts on the data to clean up the data, these can and need to be adjusted for each data set seperatly. For each particle species the dEdx signal is sliced in momentum bins and fitted with a Gaussian function. 

In line 3048 and more the particles are defined which are used for the BB fit. At the moment elecrons, pions , protons and deuterons are used. Here one can remove the deuterons again from the fit.

In the lines starting at 3731 the low momentum correction fits are performed. There you can change the range of the low momentum correction and even the function which is used for each particles species. This can be changed if the fit are not working properly and are not catching all details.

In this function also some QA plots are generated which contain:
- dEdx vs p for different particles species and selections (TPC only, TPC+TOF, V0 TPC only, V0 TPC+TOF)
- Bethe-Bloch fit as function of betagamma
- residual of Bethe-Bloch fit and used data points
- performance of the low momentum correction for electrons, pions, kaons and protons.


## extract.C

Here you can change the multiplicty range of tracks used for the BB fit. Usually only tracks from a low multiplicty environment are used (<3000 tracks) for the BB fit, since here the contamination is under control and detector effects due to large occupancy in the detector are not taken into account.

## extractMultiplicityDependence.C

Here the multiplicty dependence is extracted. This step is needed for PbPb and pPb collisions.

## correctShapeEtaTree.C and checkShapeEtaTree.C

Here the eta maps are created as function of tan(Theta) and 1/dEdx using the TTree which contains a clean proton sample. Here the selected protons can be further selected if wanted.

## checkPullTree.C

In this part of the code the the splines together with eta and mulitplicty correction are applied on the proton tree to check the performance of the splines. As QA plots the mean and sigma of the TPCnsigma distribution are created.


