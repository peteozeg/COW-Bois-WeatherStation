import os
import csv
import time

def process_csv_file():
    # delay to make sure position file created
    #time.sleep(3)
    
    # Get the directory where the script is located
    script_dir = os.path.dirname(os.path.abspath(__file__))

    manufacturing_dir = os.path.join(script_dir, 'Manufacturing')
    
    # Find CSV file with "-all-pos" in the name
    csv_file = None
    for filename in os.listdir(manufacturing_dir):
        if filename.endswith('.csv') and '-all-pos' in filename:
            csv_file = filename
            break
    
    if not csv_file:
        print("No CSV file with '-all-pos' found in the manufacturing directory.")
        return
    
    print(f"Found file: {csv_file}")
    
    # Create new filename by removing "-all-pos"
    new_filename = csv_file.replace('-all-pos', '')
    
    # Full paths
    input_path = os.path.join(manufacturing_dir, csv_file)
    output_path = os.path.join(manufacturing_dir, new_filename)
    
    # Read and process the CSV
    try:
        with open(input_path, 'r', newline='', encoding='utf-8') as infile:
            reader = csv.reader(infile)
            rows = list(reader)
        
        if not rows:
            print("CSV file is empty.")
            return
        
        # Process header (first row)
        header = rows[0]
        
        # Rename columns according to specifications
        # Assuming columns are in order: Ref, Val, Package, PosX, PosY, Rot, Side
        if len(header) < 7:
            print(f"Warning: Expected at least 7 columns, found {len(header)}")
            return
        
        # Column mapping (original positions)
        designator_col = 0  # Ref -> Designator
        mid_x_col = 3       # PosX -> Mid X
        mid_y_col = 4       # PosY -> Mid Y
        layer_col = 6       # Side -> Layer
        rotation_col = 5    # Rot -> Rotation
        
        # Create new rows with reordered columns
        new_rows = []
        
        # New header
        new_header = ["Designator", "Mid X", "Mid Y", "Layer", "Rotation"]
        new_rows.append(new_header)
        
        # Process data rows
        for row in rows[1:]:
            if len(row) < 7:
                continue  # Skip incomplete rows
            
            new_row = [
                row[designator_col],  # Designator
                row[mid_x_col],       # Mid X
                row[mid_y_col],       # Mid Y
                row[layer_col],       # Layer
                row[rotation_col]     # Rotation
            ]
            new_rows.append(new_row)
        
        # Write the new CSV
        with open(output_path, 'w', newline='', encoding='utf-8') as outfile:
            writer = csv.writer(outfile)
            writer.writerows(new_rows)
        
        print(f"Successfully processed and saved to: {new_filename}")
        print(f"Removed columns: Val, Package")
        print(f"Reordered columns: {', '.join(new_header)}")
        
        # Optionally, remove the original file
        # Uncomment the line below if you want to delete the original file
        os.remove(input_path)
        
    except Exception as e:
        print(f"Error processing file: {e}")

if __name__ == "__main__":
    process_csv_file()