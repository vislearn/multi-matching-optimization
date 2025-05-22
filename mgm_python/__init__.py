from ._pylibmgm import *
from . import solver, io, _pylibmgm
import logging

_LOGGER = logging.getLogger("libmgm")
_LOGGER.setLevel(logging.INFO)

_pylibmgm._register_api_logger(_LOGGER)
io._register_io_logger(_LOGGER)

# hide background module und functions from help() and documentation tools.
del _pylibmgm 
del io._register_io_logger