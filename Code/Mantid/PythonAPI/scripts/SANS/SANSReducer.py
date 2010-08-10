"""
    SANS-specific implementation of the Reducer. The SANSReducer class implements 
    a predefined set of reduction steps to be followed. The actual ReductionStep objects
    executed for each of those steps can be modified.
"""
from Reducer import Reducer
from Reducer import ReductionStep
import SANSReductionSteps
from mantidsimple import *

## Version number
__version__ = '0.0'

class SANSReducer(Reducer):
    """
        SANS-specific implementation of the Reducer
        #TODO: ISIS can't deal with the solidAngle() and WeightedAzimuthalAverage steps, can we make them optional and remove references to them from this file?
        #TODO: _dark_current_subtracter  isn't a spallation source concept, I don't think
    """
    ## Normalization options
    #TODO: those also correspond to the timer and monitor spectra -> store this in instr conf instead
    NORMALIZATION_NONE = None
    NORMALIZATION_TIME = 1
    NORMALIZATION_MONITOR = 0    
    
    ## Beam center finder ReductionStep object
    _beam_finder = None 
    
    ## Normalization option
    _normalizer = None
    
    ## Dark current data file
    _dark_current_subtracter = None
    
    ## Sensitivity correction ReductionStep object
    _sensitivity_correcter = None
    
    ## Solid angle correcter
    _solid_angle_correcter = None
    
    ## Azimuthal averaging
    _azimuthal_averager = None
    
    ## Transmission calculator
    _transmission_calculator = None
    
    ## Masking step
    _mask = None
    
    ## Output saving step
    _save_iq = None
    
    ## Background subtracter
    _background_subtracter = None
    
    ## Data loader
    _data_loader = None
    
    def __init__(self):
        super(SANSReducer, self).__init__()
        
        # Default beam finder
        self._beam_finder = SANSReductionSteps.BaseBeamFinder(0,0)
        
        # Default normalization
        self._normalizer = SANSReductionSteps.Normalize(SANSReducer.NORMALIZATION_TIME)
        
        # By default, we want the solid angle correction
        self._solid_angle_correcter = SANSReductionSteps.SolidAngle()
        
        # By default, use the weighted azimuthal averaging
        self._azimuthal_averager = SANSReductionSteps.WeightedAzimuthalAverage() 
        
        # Default data loader
        self._data_loader = SANSReductionSteps.LoadRun()
    
    def set_normalizer(self, option):
        """
            Set normalization option (time or monitor)
            @param option: normalization option
        """
        if option not in (None, SANSReducer.NORMALIZATION_TIME, 
                          SANSReducer.NORMALIZATION_MONITOR):
            raise RuntimeError, "SANSReducer.set_normalization: unrecognized normalization option."
        
        if option is None:
            self._normalizer = None
        else:
            self._normalizer = SANSReductionSteps.Normalize(option)
        
    def set_transmission(self, trans):
        """
             Set the transmission
             @param trans: transmission value
             @param error: uncertainty on the transmission
        """
        if issubclass(trans.__class__, SANSReductionSteps.BaseTransmission) or trans is None:
            self._transmission_calculator = trans
        else:
            raise RuntimeError, "Reducer.set_transmission expects an object of class ReductionStep"
        
    def set_mask(self, mask):
        """
            Set the reduction step that will apply the mask
            @param mask: ReductionStep object
        """
        if issubclass(mask.__class__, ReductionStep) or mask is None:
            self._mask = mask
        else:
            raise RuntimeError, "Reducer.set_mask expects an object of class ReductionStep"
        
    def get_beam_center(self): 
        """
            Return the beam center position
        """
        return self._beam_finder.get_beam_center()
    
    def set_beam_finder(self, finder):
        """
            Set the ReductionStep object that finds the beam center
            @param finder: BaseBeamFinder object
        """
        if issubclass(finder.__class__, SANSReductionSteps.BaseBeamFinder) or finder is None:
            self._beam_finder = finder
        else:
            raise RuntimeError, "Reducer.set_beam_finder expects an object of class ReductionStep"
    
    def set_data_loader(self, loader):
        """
            Set the loader for all data files
            @param loader: ReductionStep object
        """
        if issubclass(loader.__class__, ReductionStep):
            self._data_loader = loader
        else:
            raise RuntimeError, "Reducer.set_data_loader expects an object of class ReductionStep"
        
    def set_sensitivity_correcter(self, correcter):
        """
            Set the ReductionStep object that applies the sensitivity correction.
            The ReductionStep object stores the sensitivity, so that the object
            can be re-used on multiple data sets and the sensitivity will not be
            recalculated.
            @param correcter: ReductionStep object
        """
        if issubclass(correcter.__class__, ReductionStep) or correcter is None:
            self._sensitivity_correcter = correcter
        else:
            raise RuntimeError, "Reducer.set_sensitivity_correcter expects an object of class ReductionStep"
    
    def set_dark_current_subtracter(self, subtracter):
        """
            Set the ReductionStep object that subtracts the dark current from the data.
            The loaded dark current is stored by the ReductionStep object so that the
            subtraction can be applied to multiple data sets without reloading it.
            @param subtracter: ReductionStep object
        """
        if issubclass(subtracter.__class__, ReductionStep) or subtracter is None:
            self._dark_current_subtracter = subtracter
        else:
            raise RuntimeError, "Reducer.set_dark_current expects an object of class ReductionStep"
    
    def set_solid_angle_correcter(self, correcter):
        """
            Set the ReductionStep object that performs the solid angle correction.
            @param subtracter: ReductionStep object
        """
        if issubclass(correcter.__class__, ReductionStep) or correcter is None:
            self._solid_angle_correcter = correcter
        else:
            raise RuntimeError, "Reducer.set_solid_angle_correcter expects an object of class ReductionStep"
    
    def set_azimuthal_averager(self, averager):
        """
            Set the ReductionStep object that performs azimuthal averaging
            and transforms the 2D reduced data set into I(Q).
            @param averager: ReductionStep object
        """
        if issubclass(averager.__class__, ReductionStep) or averager is None:
            self._azimuthal_averager = averager
        else:
            raise RuntimeError, "Reducer.set_azimuthal_averager expects an object of class ReductionStep"
    
    def set_save_Iq(self, save_iq):
        """
            Set the ReductionStep object that saves the I(q) output
            @param averager: ReductionStep object
        """
        if issubclass(save_iq.__class__, ReductionStep) or save_iq is None:
            self._save_iq = save_iq
        else:
            raise RuntimeError, "Reducer.set_save_Iq expects an object of class ReductionStep"
    
    def set_background(self, data_file=None):
        """
            Sets the background data to be subtracted from sample data files
            @param data_file: Name of the background file
        """
        if data_file is None:
            self._background_subtracter = None
        else:
            # Check that the file exists
            self._full_file_path(data_file)
            self._background_subtracter = SANSReductionSteps.SubtractBackground(data_file)
    
    def pre_process(self): 
        """
            Reduction steps that are meant to be executed only once per set
            of data files. After this is executed, all files will go through
            the list of reduction steps.
        """
        if self._beam_finder is not None:
            self._beam_finder.execute(self)
            
        # Create the list of reduction steps
        self._to_steps()            
    
    def post_process(self): raise NotImplemented
    
    def _2D_steps(self):
        """
            Creates a list of reduction steps to be applied to
            each data set, including the background file. 
            Only the steps applied to a data set
            before azimuthal averaging are included.
        """
        reduction_steps = []
        
        # Load file
        reduction_steps.append(self._data_loader)
        
        # Dark current subtraction
        if self._dark_current_subtracter is not None:
            reduction_steps.append(self._dark_current_subtracter)
        
        # Normalize
        if self._normalizer is not None:
            reduction_steps.append(self._normalizer)
        
        # Mask
        if self._mask is not None:
            reduction_steps.append(self._mask)
        
        # Sensitivity correction
        if self._sensitivity_correcter is not None:
            reduction_steps.append(self._sensitivity_correcter)
            
        # Solid angle correction
        if self._solid_angle_correcter is not None:
            reduction_steps.append(self._solid_angle_correcter)
        
        # Calculate transmission correction
        if self._transmission_calculator is not None:
            reduction_steps.append(self._transmission_calculator) 
            
        return reduction_steps
    
    def _to_steps(self):
        """
            Creates a list of reduction steps for each data set
            following a predefined reduction approach. For each 
            predefined step, we check that a ReductionStep object 
            exists to take of it. If one does, we append it to the 
            list of steps to be executed.
        """
        # Get the basic 2D steps
        self._reduction_steps = self._2D_steps()
        
        # Subtract the background
        if self._background_subtracter is not None:
            self.append_step(self._background_subtracter)
        
        # Perform azimuthal averaging
        if self._azimuthal_averager is not None:
            self.append_step(self._azimuthal_averager)
            
        # Save output to file
        if self._save_iq is not None:
            self.append_step(self._save_iq)
            