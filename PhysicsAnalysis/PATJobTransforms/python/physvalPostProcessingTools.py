# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import yaml
import logging

crash_on_error = False

### --- HELPER --- ###
def find_histo(root_file, path, should_delete=False):
    obj = root_file.Get(path)
    if not obj:
        if crash_on_error:
            raise RuntimeError(f"Object not found at path: {path}")
        else:
            logging.warning(f"Object not found at path: {path}")
            return None
    if should_delete:
        # Extract directory and name for deletion
        path_parts = path.strip("/").split("/")
        *dirs, name = path_parts
        current_dir = root_file
        for d in dirs:
            current_dir = current_dir.Get(d)
            if not current_dir: 
                if crash_on_error:
                    raise RuntimeError(f"Directory '{d}' not found in path '{'/'.join(dirs)}' for deletion.")
                else:
                    logging.warning(f"Directory '{d}' not found in path '{'/'.join(dirs)}' for deletion.")
                    return None
        # logging.info(f"Deleting '{name}' in '{'/'.join(dirs)}'")
        current_dir.Delete(f"{name};*")
        return None
    return obj

### --- HELPER --- ###
def ensure_directory_exists(root_file, path):
    current = root_file
    for part in path.strip("/").split("/"):
        next_dir = current.Get(part)
        if not next_dir:
            logging.info(f"Saving directory not exist. Creating directory '{part}' in '{current.GetName()}'")
            current.mkdir(part)
        current.cd(part)
        current = ROOT.gDirectory

class ConfigManager:
    def __init__(self):
        self.configs = {}

    def add_configuration(self, name, config_file):
        with open(config_file, "r") as f:
            config_data = yaml.safe_load(f)
            self.configs[name] = config_data
        logging.info(f"Configuration for '{name}' loaded from {config_file}")

    def get_config(self, name):
        """Returns the config for a specific domain (e.g., 'Electron')"""
        return self.configs.get(name, {})

    def get_all_configs(self):
        """Returns all configs as (name, config) pairs"""
        return self.configs.items()

class RebinningOperation:
    def __init__(self, input, output, rebin_factor, delete_original=False):
        self.input = input 
        self.output = output
        self.rebin_factor = rebin_factor
        self.delete_original = delete_original

    def __call__(self, root_file):
        hist = find_histo(root_file, self.input)
        if not hist:
            if crash_on_error:
                raise RuntimeError(f"Histogram '{self.input}' not found for rebinning.")
            else:
                logging.error(f"Histogram '{self.input}' not found for rebinning.")
                return

        path_parts = self.output.strip("/").split("/")
        *dir_parts, output_name = path_parts
        directory_path = "/".join(dir_parts)
        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)

        if hist.GetDimension() == 1:
            rebinned = hist.Rebin(self.rebin_factor, output_name)
            rebinned.Write(output_name, ROOT.TObject.kOverwrite)
            logging.info(f"Rebinned 1D histogram and saved as '{output_name}' in '{directory_path}'.")
        elif hist.GetDimension() == 2:
            hist.RebinX(self.rebin_factor)
            hist.RebinY(self.rebin_factor)
            hist.SetName(output_name)
            hist.Write(output_name, ROOT.TObject.kOverwrite)
            logging.info(f"Rebinned 2D histogram and saved as '{output_name}' in '{directory_path}'.")
        if self.delete_original:
            find_histo(root_file, self.input, should_delete=True)

    @staticmethod
    def from_yaml(fragment):
        return RebinningOperation(
            input=fragment["input"],
            output=fragment["output"],
            rebin_factor=fragment["rebin"],
            delete_original=fragment.get("delete_original", False)
        )
    
class EfficiencyComputation:
    def __init__(self, numerator, denominator, output):
        self.numerator = numerator
        self.denominator = denominator
        self.output = output

    def __call__(self, root_file):
        num = find_histo(root_file, self.numerator)
        den = find_histo(root_file, self.denominator)
        if not num or not den:
            if crash_on_error:
                raise RuntimeError(f"Missing histogram: {self.numerator} or {self.denominator}")
            else:
                logging.error(f"Missing histogram: {self.numerator} or {self.denominator}")
                return

        if not ROOT.TEfficiency.CheckConsistency(num, den):
            logging.warning("Inconsistency detected between numerator and denominator histograms.")
            logging.info("Attempting to fix...")
            for i in range(1, num.GetNbinsX() + 1):
                num_val = num.GetBinContent(i)
                den_val = den.GetBinContent(i)
                if den_val == 0:
                    den.SetBinContent(i, 1e-6)
                    den_val = 1e-6  # avoid zero denominator by assigning a tiny epsilon
                    logging.info(f"Setting denominator to a smaller value for bin {i} as it is zero. Any other options?")
                if num_val > den_val:
                    logging.info(f"For Bin {i}: Num ({num_val}) > Den ({den_val})! Adjusting by setting Den to Num. Any other options?")
                    den.SetBinContent(i, num_val)
            if not ROOT.TEfficiency.CheckConsistency(num, den):
                logging.error("Unable to fix histogram inconsistencies. Aborting efficiency calculation.")
                return

        eff = ROOT.TEfficiency(num, den)
        path_parts = self.output.strip("/").split("/")
        *dir_path, obj_name = path_parts
        directory_path = "/".join(dir_path)
        eff.SetName(obj_name)
        eff.SetTitle(f"{self.numerator}/{self.denominator};{num.GetXaxis().GetTitle()};Efficiency")
        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)
        eff.Write()
        logging.info(f"Saved efficiency '{obj_name}' in '{directory_path}'.")

    @staticmethod
    def from_yaml(fragment):
        return EfficiencyComputation(
            numerator=fragment["numerator"],
            denominator=fragment["denominator"],
            output=fragment["output"]
        )

class ResolutionComputation:
    # Computes the s68 and s90 resolution widths for a histogram.
    def __init__(self, histogram_name):
        self.histogram_name = histogram_name

    def __call__(self, root_file):
        hist = find_histo(root_file, self.histogram_name)
        if not hist:
            if crash_on_error:
                raise RuntimeError(f"Histogram '{self.histogram_name}' not found.")
            else:
                logging.error(f"Histogram '{self.histogram_name}' not found.")
                return

        # Step 1: Compute normalized cumulative sum
        cum = [hist.GetBinContent(i) for i in range(0, hist.GetNbinsX() + 2)]
        for i in range(1, len(cum)):
            cum[i] += cum[i - 1]
        total = cum[-1] or 1.0
        cum = [x / total for x in cum]

        # Step 2: Compute smallest intervals
        s68 = self._compute_s(hist, cum, 0.683)
        s90 = self._compute_s(hist, cum, 0.90)

        logging.info(f"Resolution widths for {self.histogram_name} -> s68: {s68:.4f}, s90: {s90:.4f}")
        logging.info("Todo: Take a 2D histo, take the quantity say xaxis, make its small intervals %s", "and then compute the Resolution widths and then save as a 1D histo with quantity on xaxis and Resolution widths on yaxis")

    def _compute_s(self, hist, cum, prob):
        # Compute the smallest interval that contains the given probability.
        min_width = float("inf")
        best = (0, 0)
        for i in range(len(cum)):
            target = cum[i] + prob
            if target > 1:
                break
            for j in range(i + 1, len(cum)):
                if cum[j] >= target:
                    width = hist.GetBinCenter(j) - hist.GetBinCenter(i)
                    if width < min_width:
                        min_width = width
                        best = (i, j)
                    break
        return 0.5 * (hist.GetBinCenter(best[1]) - hist.GetBinCenter(best[0]))

    @staticmethod
    def from_yaml(fragment):
        return ResolutionComputation(
            histogram_name=fragment["histogram"]
        )

class ROCCurveComputation:
    def __init__(self, signal_path, background_path, output):
        self.signal_path = signal_path
        self.background_path = background_path
        self.output = output

    def __call__(self, root_file):
        sig = find_histo(root_file, self.signal_path)
        bkg = find_histo(root_file, self.background_path)

        if not sig or not bkg: 
            if crash_on_error:
                raise RuntimeError(f"Missing histogram: {self.signal_path} or {self.background_path}")
            else:
                logging.error(f"Missing histogram: {self.signal_path} or {self.background_path}")
                return

        graph = self.compute_roc(sig, bkg)
        if not graph:
            logging.error("ROC graph is empty or could not be constructed.")
            return

        path_parts = self.output.strip("/").split("/")
        *dir_path, obj_name = path_parts
        directory_path = "/".join(dir_path)

        graph.SetName(obj_name)
        graph.SetTitle(f"{obj_name};Signal Efficiency;Background Rejection")

        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)
        graph.Write()
        logging.info(f"Saved ROC curve '{obj_name}' in '{directory_path}'.")

    def compute_roc(self, sig, bkg):
        n_bins = sig.GetNbinsX()
        total_sig = sig.Integral()
        total_bkg = bkg.Integral()

        x_vals, y_vals = [], []
        if total_sig == 0 or total_bkg == 0:
            logging.error("Zero total signal or background counts; cannot compute ROC.")
            return None

        for cut_bin in range(1, n_bins + 1): # Loops over histogram bins, each corresponding to a cut value on the tagger output.
            sig_pass = sig.Integral(cut_bin, n_bins)
            bkg_pass = bkg.Integral(cut_bin, n_bins)

            eff = sig_pass / total_sig # Signal efficiency eff = (number of b-jets passing cut) / (total b-jets).

            if bkg_pass <= 0:
                # logging.info(f"Skipping cut bin {cut_bin} due to zero or negative background pass count: {bkg_pass}. Any other options?")
                continue

            rej = total_bkg / bkg_pass # Background rejection rej = (total background jets) / (number of background jets passing cut).

            x_vals.append(eff)
            y_vals.append(rej)

        if not x_vals:
            return None

        graph = ROOT.TGraphErrors(len(x_vals))
        for i, (x, y) in enumerate(zip(x_vals, y_vals)):
            graph.SetPoint(i, x, y)

        return graph

    @staticmethod
    def from_yaml(fragment):
        return ROCCurveComputation(
            signal_path=fragment["signal"],
            background_path=fragment["background"],
            output=fragment["output"]
        )
    
class ProjectionComputation:
    def __init__(self, input_hist, bins_x, bins_y, projection_axis, output=None):
        self.input_hist = input_hist
        self.bins_x = bins_x
        self.bins_y = bins_y
        self.x_proj = projection_axis == "x"
        self.y_proj = projection_axis == "y"
        self.output = output

    def __call__(self, root_file):
        hist = find_histo(root_file, self.input_hist)
        if not hist:
            if crash_on_error:
                raise RuntimeError(f"Histogram '{self.input_hist}' not found.")
            else:
                logging.error(f"Histogram '{self.input_hist}' not found.")
                return

        b1, b2 = hist.GetYaxis().FindBin(self.bins_x[0]), hist.GetYaxis().FindBin(self.bins_x[1])
        b3, b4 = hist.GetXaxis().FindBin(self.bins_y[0]), hist.GetXaxis().FindBin(self.bins_y[1])

        path_parts = self.output.strip("/").split("/")
        *dir_path, output_name = path_parts
        directory_path = "/".join(dir_path)
        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)

        if self.x_proj:
            proj_x = hist.ProjectionX(output_name, b1, b2).Clone(output_name)
            self.project(proj_x, hist.GetXaxis().GetTitle(), "X")
        if self.y_proj:
            proj_y = hist.ProjectionY(output_name, b3, b4).Clone(output_name)
            self.project(proj_y, hist.GetYaxis().GetTitle(), "Y")

    def project(self, projection, axis_title, axis):
        projection.SetTitle(f";{axis_title};Entries")
        projection.SetStats(False)
        projection.Write()
        logging.info(f"Saved projection by integrating over {axis} axis as '{projection.GetName()}'.")

    @staticmethod
    def from_yaml(fragment):
        return ProjectionComputation(
            input_hist=fragment["hist"],
            bins_x=fragment.get("bins_projection_x", [10, 200]),
            bins_y=fragment.get("bins_projection_y", [-2.47, 2.47]),
            projection_axis=fragment.get("projection_axis", "x"),
            output=fragment.get("output")
        )

class HistogramAddition:
    def __init__(self, histo1, histo2, output, delete_inputs=False):
        self.histo1 = histo1
        self.histo2 = histo2
        self.output = output
        self.delete_inputs = delete_inputs

    def __call__(self, root_file):
        histo1 = find_histo(root_file, self.histo1)
        histo2 = find_histo(root_file, self.histo2)
        if not histo1 or not histo2:
            if crash_on_error:
                raise RuntimeError(f"Missing histograms: {self.histo1} or {self.histo2}")
            else:
                logging.error(f"Missing histograms: {self.histo1} or {self.histo2}")
                return

        path_parts = self.output.strip("/").split("/")
        *dir_path, output_name = path_parts
        directory_path = "/".join(dir_path)
        summed = histo1.Clone(output_name)
        summed.Add(histo2)
        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)
        summed.Write(output_name, ROOT.TObject.kOverwrite)
        logging.info(f"Saved added histogram '{output_name}' in '{directory_path}'.")
        if self.delete_inputs:
            # logging.info(f"Deleting input histograms '{self.histo1}' and '{self.histo2}' after addition.")
            find_histo(root_file, self.histo1, should_delete=True)
            find_histo(root_file, self.histo2, should_delete=True)

    @staticmethod
    def from_yaml(fragment):
        return HistogramAddition(
            histo1=fragment["histo1_name"],
            histo2=fragment["histo2_name"],
            output=fragment["output"],
            delete_inputs=fragment.get("delete_inputs", False)
        )

class HistogramAdjustment:
    def __init__(self, input_hist, new_path, x_label=None, y_label=None, xlow=None, xhigh=None):
        self.input_hist = input_hist
        self.new_path = new_path
        self.x_label = x_label
        self.y_label = y_label
        self.xlow = xlow
        self.xhigh = xhigh

    def __call__(self, root_file):
        ROOT.TH1.AddDirectory(False)
        hist = find_histo(root_file, self.input_hist)
        if not hist:
            if crash_on_error:
                raise RuntimeError(f"Histogram '{self.input_hist}' not found.")
            else:
                logging.error(f"Histogram '{self.input_hist}' not found.")
                return

        path_parts = self.new_path.strip("/").split("/")
        *dir_path, new_name = path_parts
        directory_path = "/".join(dir_path)
        ensure_directory_exists(root_file, directory_path)
        root_file.cd(directory_path)

        if self.xlow is None or self.xhigh is None:
            h_out = hist.Clone(new_name)
        else:
            ax = hist.GetXaxis()
            bin_low = max(1, min(ax.FindBin(self.xlow), ax.GetNbins()))
            bin_high = max(1, min(ax.FindBin(self.xhigh), ax.GetNbins()))
            nbins = bin_high - bin_low
            h_out = ROOT.TH1D(new_name, hist.GetTitle(), nbins + 1, float(self.xlow), float(self.xhigh))
            for i in range(nbins + 1):
                h_out.SetBinContent(i + 1, hist.GetBinContent(i + bin_low))

        if self.x_label:
            h_out.GetXaxis().SetTitle(self.x_label)
        if self.y_label:
            h_out.GetYaxis().SetTitle(self.y_label)
        h_out.Write(new_name, ROOT.TObject.kOverwrite)
        logging.info(f"Renamed and updated histogram '{self.input_hist}' → '{self.new_path}' in '{directory_path}' | X-axis: '{self.x_label}' | Y-axis: '{self.y_label}' | Range: ({self.xlow}, {self.xhigh})")

    @staticmethod
    def from_yaml(fragment):
        return HistogramAdjustment(
            input_hist=fragment["hist"],
            new_path=fragment.get("final_histo_name", fragment["hist"]),
            x_label=fragment.get("x_axis_label"),
            y_label=fragment.get("y_axis_label"),
            xlow=fragment.get("xlow"),
            xhigh=fragment.get("xhigh")
        )
    
class HistogramBlackList:
    def __init__(self, toDelete):
        self.toDelete = toDelete

    def __call__(self, root_file):
        find_histo(root_file, self.toDelete, should_delete=True)

    @staticmethod
    def from_yaml(fragment):
        return HistogramBlackList(
            toDelete=fragment["toDelete"]
        )
