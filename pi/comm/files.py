import json

def read_data(path):
    try:
        with open(path, "r") as file:
            data = json.load(file)
            return data
    except FileNotFoundError:
        print(f"No file found at {path}")
    except json.JSONDecodeError:
        print("Error decoding JSON.")
    return None

def save_data(data, path):
    try:
        with open(path, "w") as file:
            json.dump(data, file, indent=4) 
            return True
    except FileNotFoundError:
        print(f"No file found at {path}")
    return False
