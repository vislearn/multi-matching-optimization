from ._pylibmgm import *
from ._pylibmgm import _register_api_logger
from . import solver, io
import logging

_LOGGER = logging.getLogger("libmgm")
_LOGGER.setLevel(logging.INFO)

_register_api_logger(_LOGGER)
io._register_io_logger(_LOGGER)