from ._pylibmgm import *
from . import solver, io
import logging

LOGGER = logging.getLogger("libmgm")
LOGGER.setLevel(logging.INFO)

register_api_logger(LOGGER)
io.register_io_logger(LOGGER)