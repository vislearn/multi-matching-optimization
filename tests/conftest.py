import pylibmgm
import pytest
import logging

from pathlib import Path

@pytest.fixture(scope="session", autouse=True)
def set_logger():
    logging.getLogger("libmgm").setLevel(logging.WARNING)
    logging.getLogger("libmgm.interface").setLevel(logging.WARNING)


# Optimum is complete matching
synthetic_complete_4 = "synthetic_complete_instance_1_nNodes_10_nGraphs_4.txt"

# Optimum is incomplete matching
hotel_4 = "hotel_instance_1_nNodes_10_nGraphs_4.txt"
house_8 = "house_instance_1_nNodes_10_nGraphs_8.txt"

# Sparse problem
worms_3 = "worms_3_small.dd"

# GM problem
opengm = "opengm1.dd"

@pytest.fixture(scope="session")
def synth_4_model():
    model_path = Path(__file__).parent / synthetic_complete_4
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session")
def hotel_4_model():
    model_path = Path(__file__).parent / hotel_4
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def house_8_model():
    model_path = Path(__file__).parent / house_8
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def worms_3_model():
    model_path = Path(__file__).parent / worms_3
    model = pylibmgm.io.parse_dd_file(model_path)
    return model

@pytest.fixture(scope="session") 
def opengm_model():
    model_path = Path(__file__).parent / opengm
    model = pylibmgm.io.parse_dd_file_gm(model_path)
    return model