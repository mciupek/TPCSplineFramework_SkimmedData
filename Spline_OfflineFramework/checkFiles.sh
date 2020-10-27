#!/bin/bash

# ===| check if element is contained in an array |==============================
# param1: element to be checked for
# param2: array to search in
#
# example:
# array=("one" "two" "three")
# containsElement "one" "${array[@]}" && echo yes
# NOTE: the "" around the array expansion are required
containsElement ()
{
  local e match="$1"
  shift
  for e; do [[ "$e" =~ ^$match$ ]] && return 0; done
  return 1
}

# ==============================================================================
# main
#

# ===| basic configuration |====================================================

# spline base directory in alien
alien_base_dir=/alice/cern.ch/user/t/tpcdrop/calib/Splines

# patter to extract the iteration used from README.txt
iteration_pattern="Used Iteration:"

# patter to extract the iteration used from README.txt
qa_iteration_pattern="QA subfolder:"

# ===| arrays with the expected file names |====================================

# file for the iteration used
expected_files=(
betheblochresidual_SaturatedLund\.pdf
bethebloch_SaturatedLund\.pdf
bhess_PIDetaTree_recalcdEdx_checkPullSigma_.*\.root
CompareSplines.*\.pdf
checkPullTree_mean\.pdf
checkPullTree_sigma\.pdf
outputCheckShapeEtaTree_[0-9_]{17}__CorrectedWithMap_outputCheckShapeEtaTree_.*\.root
outputCheckShapeEtaTree_[0-9_]{17}__NewSplines__.*\.root
polynomials0\.pdf
polynomials1\.pdf
polynomials2\.pdf
polynomials3\.pdf
splines\.root
splines_QA_arrGraphs_check_jy\.root
splines_QA_BetheBlochFit_SaturatedLund\.root
splines_QA_ResidualGraphsTOF\.root
splines_QA_ResidualGraphsTPC\.root
splines_QA_ResidualGraphsV0\.root
splines_QA_ResidualGraphsV0plusTOF\.root
splines_QA_ResidualPolynomials\.root
V0_dEdx_purity_el\.root
V0_dEdx_purity_Pi\.root
V0_dEdx_purity_Pr\.root
)

# files for the QA iteration
expected_files_QA=(
betheblochresidual_SaturatedLund\.pdf
bethebloch_SaturatedLund\.pdf
bhess_PIDetaTree_recalcdEdx_checkPullSigma_.*\.root
CompareSplines.*\.pdf
checkPullTree_mean\.pdf
checkPullTree_sigma\.pdf
outputCheckShapeEtaAdv_.*.root
PIDqaReport\.pdf
polynomials0\.pdf
polynomials1\.pdf
polynomials2\.pdf
polynomials3\.pdf
splines\.root
splines_QA_arrGraphs_check_jy\.root
splines_QA_BetheBlochFit_SaturatedLund\.root
splines_QA_ResidualGraphsTOF\.root
splines_QA_ResidualGraphsTPC\.root
splines_QA_ResidualGraphsV0\.root
splines_QA_ResidualGraphsV0plusTOF\.root
splines_QA_ResidualPolynomials\.root
V0_dEdx_purity_el\.root
V0_dEdx_purity_Pi\.root
V0_dEdx_purity_Pr\.root
)

# ==============================================================================
# loop over all Splines folders and check structure

for folder in $(alien_ls -n -F /alice/cern.ch/user/t/tpcdrop/calib/Splines/ | grep '/$'); do
  echo 
  echo "========================================================================="
  echo "===|    Checking folder '$folder' "
  echo

  spline_folder=${alien_base_dir}/${folder}

  # ---| test number of entries in the main folder |---
  main_folder=($(alien_ls $spline_folder))
  if [[ ${#main_folder[*]} -ne 3 ]]; then
    echo "Number of file in the main folder is different from 3"
    #for f in ${main_folder[*]}; do 
      #echo $f
    #done 
    echo
  fi

  # ---| test for README.txt |---
  readme_file=$spline_folder/README.txt
  if ! containsElement README\.txt "${main_folder[@]}" &> /dev/null; then
    echo "README.txt file is missing"
    echo "skipping this directory"
    continue
  fi

  # ---| check for two folder starting with 'Iteration' |---
  start_with_Iteration=$(for f in ${main_folder[*]}; do echo $f | grep "^Iteration[0-9]"; done | wc -l)
  if [[ $start_with_Iteration -ne 2 ]]; then
    echo "Found $start_with_Iteration folders beginning with 'Iteration' instead of 2"
    echo
  fi

  # ---| copy README.txt and get Iteration and QA Iteration |---
  tmp=$(mktemp)
  alien_cp alien:$readme_file file:$tmp &> /dev/null
  iterationNr=$(grep "$iteration_pattern" $tmp | awk '{print $NF}')
  iterationQA=$(grep "$qa_iteration_pattern" $tmp | awk '{print $NF}')

  if [[ ! "$iterationNr" =~ ^Iteration ]]; then
    echo "Could not extract iteration number for splines from README.txt"
    echo "skipping this directory"
    rm $tmp
    continue
  fi

  if [[ ! "$iterationQA" =~ ^Iteration ]]; then
    echo "Could not extract iteration number for QA from README.txt"
    echo "skipping this directory"
    rm $tmp
    continue
  fi

  echo "Found spline iteration: $iterationNr"
  echo "Found QA iteration:     $iterationQA"
  echo

  # ---| check spline iteration files |---
  echo "------------------------------------------------------------------------"
  echo "Checking spline iteration ($iterationNr)"
  echo
  splineIterationFiles=($(alien_ls $spline_folder/${iterationNr}))

  # check number of files
  if [[ ${#expected_files[*]} -ne ${#splineIterationFiles[*]} ]]; then
    echo "Expected number of files (${#expected_files[*]}) and found number of files (${#splineIterationFiles[*]}) differ"
    echo
  fi

  # check individual files
  for pattern in ${expected_files[@]}; do
    if ! containsElement "$pattern" "${splineIterationFiles[@]}"; then
      echo "File '$pattern' is missing"
    fi
  done
  echo

  # ---| check QA iteration files |---
  echo "------------------------------------------------------------------------"
  echo "Checking QA iteration ($iterationQA)"
  echo
  splineIterationFilesQA=($(alien_ls ${alien_base_dir}/${folder}/${iterationQA}))

  # check number of files
  if [[ ${#expected_files_QA[*]} -ne ${#splineIterationFilesQA[*]} ]]; then
    echo "Expected number of files (${#expected_files[*]}) and found number of files (${#splineIterationFiles[*]}) differ"
    echo
  fi


  # check individual files
  for pattern in ${expected_files_QA[@]}; do
    if ! containsElement "$pattern" "${splineIterationFilesQA[@]}"; then
      echo "File '$pattern' is missing"
    fi
  done

  # ---| clean up |---
  rm $tmp
done
