import pylibmgm
import pytest
import logging

@pytest.fixture(scope="session", autouse=True)
def set_logger():
    logging.getLogger("libmgm").setLevel(logging.WARNING)
    logging.getLogger("libmgm.interface").setLevel(logging.WARNING)