#!/usr/bin/env python3
"""Prove explicit WMA left-channel source preserves all verified executable bytes."""
import datetime,json
from pe import PE
from record_live_production_verification import ROOT,digest,read,write

def main():
    prior=read(ROOT/'analysis/squall-mpeg-regression.json');old=ROOT/'.local/squall-wma-before';changed='src/squall/runtime/SoundFile/Wma/WmaDecoder.cpp';current={}
    for name,value in prior['production_source_files'].items():
        path=old/'WmaDecoder.cpp' if name==changed else ROOT/name;assert digest(path)==value,name;current[name]=digest(ROOT/name)
    before=(old/'WmaDecoder.cpp').read_text(encoding='utf-8');after=(ROOT/changed).read_text(encoding='utf-8')
    replacement='''                // Original1001558e reads the left WORD, doubles/divides it,
                // and advances by both channels. Preserve that observed result
                // explicitly; the public expression had unsequenced increments.
                int val = *src;
                src += 2;'''
    assert after==before.replace('                int val = (int) (*src++ + *src++) / 2;',replacement,1)
    rows=[];hashes=[]
    for index,(kind,build) in enumerate([('main','build/windows'),('fresh','build/squall-source-repro-vs20261010')]):
        for name in ['Squall.dll','Dobr\u00fd farm\u00e1\u0159.exe','JNGLoad.dll']:
            original=old/kind/name;candidate=ROOT/build/'Release'/name;pe=PE(candidate);a=original.read_bytes();b=candidate.read_bytes();assert len(a)==len(b)
            mask=[[pe.u32(0x3c)+8,pe.u32(0x3c)+12],[pe.offset(pe.directory(6)[0])+4,pe.offset(pe.directory(6)[0])+8]];assert pe.directory(6)[1]==28
            differences=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y];assert not any(not any(first<=i<last for first,last in mask) for i in differences)
            if name!='Squall.dll':assert a==b
            else:assert digest(original)==prior['candidate_sha256'][index] and digest(candidate)==digest(ROOT/build/'squall-candidate/Release/Squall.dll');hashes.append(digest(candidate))
            rows.append(dict(kind=kind,name=name,before_path=original.relative_to(ROOT).as_posix(),before_sha256=digest(original),after_path=candidate.relative_to(ROOT).as_posix(),after_sha256=digest(candidate),bytes=len(a),different_bytes=len(differences),permitted_linker_timestamp_ranges=mask,all_other_bytes_identical=True,files_modified_for_comparison=False))
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159'/row['path'])==row['sha256']
    observations=read(ROOT/'analysis/squall-wma-initial-observations.json');assert observations['matched_guarded_destinations']==96 and observations['shared_unhandled_AV_pairs']==24
    result=dict(status='explicit_original_left_channel_cpp_matches_previous_verified_program_bytes_both_builds',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),production_source_files=current,candidate_sha256=hashes,program_byte_comparisons=rows,build_logs={name:digest(ROOT/'.local'/name) for name in ['squall-wma-main-build.log','squall-wma-fresh-build.log','squall-wma-explicit-main-build.log','squall-wma-explicit-fresh-build.log']},original_instruction='1001558e MOVSX left;SHL1;signedDIV2;advance4 bytes',semantic_source_change='Replace unsequenced public C++ increments with explicit original left channel; compiled runtime code remains identical.',baseline_full_regression='analysis/squall-mpeg-regression.json',baseline_full_ctest_tests=102,baseline_full_ctest_elapsed_seconds=543.07,full_ctest_repeated_for_source_clarification=False,baseline_regression_transferred_by_complete_executable_byte_equality=True,prior_wma_observations_report='analysis/squall-wma-initial-observations.json',prior_wma_observations_sha256=digest(ROOT/'analysis/squall-wma-initial-observations.json'),original_files_unchanged=17,whole_squall_complete=False,whole_game_complete=False,pending=['Stateful WMA reads and actual prepared/playback buffer work in progress; short48kHz buffer nonprogress unresolved','All remaining original game,complete DLL,JNG and uninstall requirements'])
    write(ROOT/'analysis/squall-wma-explicit-left-validation.json',result)
    provenance=read(ROOT/'analysis/squall-runtime-provenance.json');provenance['instruction_backed_patches']=[row for row in provenance['instruction_backed_patches'] if row.get('path')!='SoundFile/Wma/WmaDecoder.cpp'];provenance['instruction_backed_patches'].append(dict(path='SoundFile/Wma/WmaDecoder.cpp',method='Explicit native stereo-to-mono left channel',original_addresses=['1001558e','10015596'],instruction_evidence='analysis/squall-wma-native-data-instructions.txt',source_difference='Unsequenced public C++ increments replaced with signed left sample and advance2; both compiled DLL bodies unchanged.',current_sha256=digest(ROOT/changed),verification_report='analysis/squall-wma-explicit-left-validation.json'));write(ROOT/'analysis/squall-runtime-provenance.json',provenance)
    print('Explicit WMA left-channel source verified:both full DLL bytes unchanged outside linker timestamps; both game EXEs and JNG DLLs exactly unchanged.')
if __name__=='__main__':main()
