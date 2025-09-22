# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ...decorators import timed
from ...utility import LHE
from xml.etree import ElementTree
import glob
import math
import re
import shutil
import sys


logger = Logging.logging.getLogger("PowhegControl")

@timed("LHE file updating event weights generated with ubexcess_correct feature")
def lhe_ubexcess_correct_reweighter(powheg_LHE_output):
	#get ubexcess correction factors for btilde and remnant events from st4 pwgcounter files
	btilde_factor = get_ub_correction_factor("btilde")
	remnant_factor = get_ub_correction_factor("remn")
	reg_factor = get_ub_correction_factor("reg")

	## Regex to extract floats :: from perldoc perlretut
	regex_match_floats = r"([+-]? *(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)"
	# [+-]? *           # first, match an optional sign (and space)
	# (?:               # then match integers or f.p. mantissas:
	#   \d+             # start out with a ...
	#   (?:
	#     \.\d*         # mantissa of the form a.b or a.
	#   )?              # ? takes care of integers of the form a
	#  |\.\d+           # mantissa of the form .b
	# )
	# (?:[eE][+-]?\d+)? # finally, optionally match an exponent

	wgt_pattern = re.compile(r"<wgt id='(\d+)'>"+regex_match_floats+r"</wgt>")

	powheg_LHE_updated = "{}.updated".format(powheg_LHE_output)
	with open(powheg_LHE_updated, "w") as f_output:
		has_eventweights = preamble_has_additional_eventweights(powheg_LHE_output)
		if not has_eventweights:
			new_preamble = preamble_with_added_nominal_eventweight(powheg_LHE_output)
			f_output.write(new_preamble)
		else:
			new_preamble = LHE.preamble(powheg_LHE_output)
			f_output.write(new_preamble)
		for input_event in LHE.event_iterator(powheg_LHE_output):
			factor = -1
			# check if event is a btilde (rwgt = 1) or remnant (rwgt = 2) event or regular (rwgt = 3)
			# and chose the corresponding correction factor
			for line in input_event.splitlines():
				if '#rwgt' in line:
					evt_type = line.split()[1]
					if evt_type == '1':
						factor = btilde_factor
					elif evt_type == '2':
						factor = remnant_factor
					elif evt_type == '3':
						factor = reg_factor
					else:
						logger.error("Event is neither a btilde nor remnant event, exiting!")
						sys.exit()
			if factor < 0:
				logger.error("Something went wrong, ubexcess_rwgt_factor below 0! Aborting")
				sys.exit()
			# multiply original event weights with correction factor
			modified_event = ""
			if not has_eventweights:
				xwgtup = get_xwgtup_value(input_event)
				# get modified nominal weight by multiply xwgtup with ubexcess correction factor
				nominal_wgt = xwgtup * factor 
				# find line after #rwgt and before #pdf
				# and add additional nominal wgt 
				for line in input_event.splitlines():
					if line.find("#pdf") >= 0:
						modified_event += "<rwgt>\n"
						modified_event += f"<wgt id='0'>{nominal_wgt:.5E}</wgt>\n"
						modified_event += "</rwgt>\n"
					modified_event+= line + "\n"
			else:
				for line in input_event.splitlines():
					match = wgt_pattern.search(line)
					if match:
						wgt_id = match.group(1)  # Extract the weight id
						value = float(match.group(2))  # Extract and convert the event weight
						new_value = value * factor  # Multiply by the correction factor
						# Format new line with modified event weight
						modified_line = f"<wgt id='{wgt_id}'>{new_value:.5E}</wgt>"
						modified_event+= modified_line + "\n"
					else:
						modified_event+= line + "\n"
			f_output.write(modified_event)
		#XWGTUP value needs to be changed, but this is done by calling lhe_nominal_weight_updater in postprocessor chain
		f_output.write(LHE.postamble(powheg_LHE_output))


	shutil.move(powheg_LHE_output, "{}.lhe_ubexcess_correct_reweighter_backup".format(powheg_LHE_output))
	shutil.move(powheg_LHE_updated, powheg_LHE_output)



# using slightly modified code from https://gitlab.cern.ch/tjezo/powheg-box-res-bb4l-sl-beta/-/blob/master/Scripts/FindReweightFromCounters.py
def get_ub_correction_factor(which):
	file_names = sum([glob.glob(_f) for _f in ["pwgcounters-st4-*.dat"]], [])
	if not file_names: 
		raise IOError("The pwgcounter files of stage 4 could not be found. Probably POWHEG-BOX crashed during event generation.")
	tot_numpts=0
	for file_name in file_names:
		# Upper bound violations [in inclusive cross-section and generation of radiation]
		with open(file_name, "r") as counterfile:
			fullline=counterfile.read()
			match=re.search('^ *'+which+' cross section used:[ =]*([^$ ]*) *$',fullline,re.M)
			if match is None:
				logger.warning(file_name+' does not contain a '+which+' cross section used')
				logger.warning('This can be fine, if there are no '+which+' events. This is being checked later.')
				return -1
			sig_used=float(match.group(1))
			match=re.search('^ *'+which+' cross section estimate:[ =]*([^$ ]*) *$',fullline,re.M)
			if match is None:
				logger.warning(file_name+' does not contain a '+which+' cross section estimate')
				logger.warning('This can be fine, if there are no '+which+' events. This is being checked later.')
				return -1
			sig_estim=float(match.group(1))
			match=re.search('^ *'+which+' cross section error estimate:[ =]*([^$ ]*) *$',fullline,re.M)
			if match is None:
				logger.warning(file_name+' does not contain a '+which+' cross section error estimate')
				logger.warning('This can be fine, if there are no '+which+' events. This is being checked later.')
				return -1
			sig_estimerr=float(match.group(1))
			match=re.search('^ *'+which+' cross section estimate num. points:[ =]*([^$ ]*) *$',fullline,re.M)
			if match is None:
				logger.warning(file_name+' does not contain a '+which+' cross section estimate num. points')
				logger.warning('This can be fine, if there are no '+which+' events. This is being checked later.')
				return -1
			sig_numpts=float(match.group(1))
			match=re.search('^ *'+which+' bound violation correction factor:[ =]*([^$ ]*) *$',fullline,re.M)
			if match is not None:
				sig_ubcorr=float(match.group(1))
			else:
				sig_ubcorr=1
				sig_numpts=float(match.group(1))
			if  tot_numpts == 0 :
				tot_numpts=sig_numpts
				tot_estim=sig_estim
				tot_estimerr=sig_estimerr
				tot_used=sig_used
				tot_ubcorr=sig_ubcorr
				tot_nfiles=1
			else:
				if tot_used != sig_used:
					logger.error("the used cross section in the current file",
					"is inconsistent with the previous ones, exiting ...")
					sys.exit()
		
				sumsq=(tot_estimerr**2*tot_numpts+tot_estim**2)*tot_numpts + \
				(sig_estimerr**2*sig_numpts+sig_estim**2)*sig_numpts
				tot_estim=(tot_estim*tot_numpts+sig_estim*sig_numpts)/(tot_numpts+sig_numpts)
				tot_numpts=tot_numpts+sig_numpts
				tot_estimerr=math.sqrt((sumsq/tot_numpts-tot_estim**2)/tot_numpts)
				if tot_ubcorr != 1 or sig_ubcorr != 1:
					tot_ubcorr=(tot_ubcorr*tot_nfiles+sig_ubcorr)/(tot_nfiles+1)
					tot_nfiles=tot_nfiles+1
				
				
	logger.info("Cross section used for {} event generation: {}".format(which,tot_used))
	logger.info("Cross section computed on the fly for {} event generation: {}+-{}".format(which,tot_estim,tot_estimerr))
	if tot_ubcorr != 1:
		logger.info("Weight increment factor due to corrections for upper bound violation in '{} events: {}".format(which,tot_ubcorr))
		logger.info("Total correction factor for {} events: {}".format(which,tot_estim/tot_used/tot_ubcorr))

	return tot_estim/tot_used/tot_ubcorr

def preamble_has_additional_eventweights(powheg_LHE_output):
	preamble = LHE.preamble(powheg_LHE_output)
	for line in preamble.splitlines():
		if line.find("<initrwgt>") >= 0:
			return True
	return False

def preamble_with_added_nominal_eventweight(powheg_LHE_output):
	original_preamble = LHE.preamble(powheg_LHE_output)
	header_elementtree = LHE.add_weight_to_header(LHE.header_block(powheg_LHE_output),"nominal","nominal","0")
	header_str_bytestr = ElementTree.tostring(header_elementtree)
	header = header_str_bytestr.decode('utf-8').replace("\"", "\'").strip() + "\n"
	#insert header befor <init> block
	preamble = ""
	for input_line in original_preamble.splitlines():
		if input_line.find("<init>") >= 0:
			preamble += header
		preamble += input_line + "\n"
	return preamble

def get_xwgtup_value(input_event):
	# do this as done in LHE.update_XWGTUP_with_reweighted_nominal
	XWGTUP = None
	for input_line in input_event.splitlines(True):
		if XWGTUP is None: # XWGTUP not yet found
			try:  # interpret line as a general event info line
				NUP, IDPRUP, XWGTUP, SCALUP, AQEDUP, AQCDUP = input_line.split()
			except ValueError:  # this is not a general event info line
				pass
	return float(XWGTUP)
