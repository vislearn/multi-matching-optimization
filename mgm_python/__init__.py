from ._pylibmgm import *
from ._pylibmgm import _register_api_logger
from . import solver, io
import logging

LOGGER = logging.getLogger("libmgm")
LOGGER.setLevel(logging.INFO)

_register_api_logger(LOGGER)
io._register_io_logger(LOGGER)