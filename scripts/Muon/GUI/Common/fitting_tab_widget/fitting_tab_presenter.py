# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from Muon.GUI.Common.fitting_tab_widget.workspace_selector_view import WorkspaceSelectorView
from Muon.GUI.Common.observer_pattern import GenericObserver


class FittingTabPresenter(object):
    def __init__(self, view, model, context):
        self.view = view
        self.model = model
        self.context = context
        self._selected_data = []
        self._start_x = [self.view.start_time]
        self._end_x = [self.view.end_time]
        self.manual_selection_made = False
        self.update_selected_workspace_guess()
        self.gui_context_observer = GenericObserver(self.update_selected_workspace_guess)
        self.run_changed_observer = GenericObserver(self.update_selected_workspace_guess)

    def handle_select_fit_data_clicked(self):
        selected_data, dialog_return = WorkspaceSelectorView.get_selected_data(self.context.data_context.current_runs,
                                                                               self.context.data_context.instrument,
                                                                               self.selected_data,
                                                                               self.view.fit_to_raw,
                                                                               self.context,
                                                                               self.view)

        if dialog_return:
            self.selected_data = selected_data
            self.manual_selection_made = True

    def update_selected_workspace_guess(self):
        if not self.manual_selection_made:
            guess_selection = self.context.get_names_of_workspaces_to_fit(runs='All',
                                                                          group_and_pair='All', phasequad=True,
                                                                          rebin=not self.view.fit_to_raw)
            self.selected_data = guess_selection

    def handle_display_workspace_changed(self):
        fit_type = self.view.fit_type

        if fit_type == 'Single Fit':
            self.view.set_datasets_in_function_browser([self.view.display_workspace])
        elif fit_type == 'Sequential Fit':
            current_index = self.view.parameter_display_combo.currentIndex()
            current_index = current_index if current_index != -1 else 0
            self.view.set_datasets_in_function_browser([self.view.display_workspace])
            self.view.start_time = self.start_x[current_index]
            self.view.end_time = self.end_x[current_index]
        else:
            current_index = self.view.parameter_display_combo.currentIndex()
            current_index = current_index if current_index != -1 else 0
            self.view.function_browser.setCurrentDataset(current_index)
            self.view.start_time = self.start_x[current_index]
            self.view.end_time = self.end_x[current_index]

    def handle_use_rebin_changed(self):
        if not self.view.fit_to_raw and not self.context._do_rebin():
            self.view.fit_to_raw = True
            self.view.warning_popup('No rebin options specified')
            return

        if not self.manual_selection_made:
            self.update_selected_workspace_guess()
        else:
            self.selected_data = self.context.get_list_of_binned_or_unbinned_workspaces_from_equivalents(
                self.selected_data)

    def handle_fit_type_changed(self):
        fit_type = self.view.fit_type

        if fit_type == 'Single Fit':
            self.view.select_workspaces_to_fit_button.setEnabled(False)
            self.view.workspace_combo_box_label.setText('Select Workspace')
            guess_selection = self.context.get_names_of_workspaces_to_fit(runs='All',
                                                                          group_and_pair='All', phasequad=True,
                                                                          rebin=not self.view.fit_to_raw)
            self.selected_data = guess_selection
        else:
            self.view.select_workspaces_to_fit_button.setEnabled(True)
            self.view.workspace_combo_box_label.setText('Display parameters for')
            guess_selection = self.context.get_names_of_workspaces_to_fit(runs='All',
                                                                          group_and_pair=self.context.gui_context[
                                                                              'selected_group_pair'], phasequad=True,
                                                                          rebin=not self.view.fit_to_raw)
            self.selected_data = guess_selection

    def handle_fit_clicked(self):
        fit_type = self.view.fit_type

        if fit_type == 'Single Fit':
            single_fit_parameters = self.get_parameters_for_single_fit()
            self.model.do_single_fit(single_fit_parameters)
        elif fit_type == 'Simultaneous Fit':
            simultaneous_fit_parameters = self.get_multi_domain_fit_parameters()
            self.model.do_simultaneous_fit(simultaneous_fit_parameters)
        elif fit_type == 'Sequential Fit':
            sequential_fit_parameters = self.get_multi_domain_fit_parameters()
            self.model.do_sequential_fit(sequential_fit_parameters)

    def handle_start_x_updated(self):
        value = self.view.start_time
        index = self.view.parameter_display_combo.currentIndex()
        self.update_start_x(index, value)

    def handle_end_x_updated(self):
        value = self.view.end_time
        index = self.view.parameter_display_combo.currentIndex()
        self.update_end_x(index, value)

    def get_parameters_for_single_fit(self):
        params = {}

        params['Function'] = self.view.fit_string
        params['InputWorkspace'] = self.view.display_workspace
        params['Minimizer'] = self.view.minimizer
        params['StartX'] = self.start_x[0]
        params['EndX'] = self.end_x[0]
        params['EvaluationType'] = self.view.evaluation_type

        return params

    def get_multi_domain_fit_parameters(self):
        params = {}

        params['Function'] = self.view.fit_string
        params['InputWorkspace'] = self.selected_data
        params['Minimizer'] = self.view.minimizer
        params['StartX'] = self.start_x
        params['EndX'] = self.end_x
        params['EvaluationType'] = self.view.evaluation_type

        return params

    @property
    def selected_data(self):
        return self._selected_data

    @selected_data.setter
    def selected_data(self, value):
        self._selected_data = value
        self.view.update_displayed_data_combo_box(value)

        if self.view.fit_type != 'Single Fit':
            self._start_x = [self.view.start_time] * len(value)
            self._end_x = [self.view.end_time] * len(value)
        if self.view.fit_type == 'Simultaneous Fit':
            self.view.set_datasets_in_function_browser(self._selected_data)

    @property
    def start_x(self):
        return self._start_x

    @property
    def end_x(self):
        return self._end_x

    def update_start_x(self, index, value):
        self._start_x[index] = value

    def update_end_x(self, index, value):
        self._end_x[index] = value
