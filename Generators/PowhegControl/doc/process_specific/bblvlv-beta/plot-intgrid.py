import matplotlib.pyplot as plt

def parse_file_contents(file_content,file_name):
    # Initialize empty lists for x and y values
    x_values = []
    y_values = []
    title = ""

    # Split the file content into lines
    lines = file_content.strip().split('\n')

    # Iterate through each line
    for line in lines:
        if line.strip().startswith("join"):
            if title == "":
                continue
            print("saving plot "+title)
            new_x = range(len(x_values))
            new_x = [i/(len(x_values)-1) for i in new_x]
            plt.plot(x_values,y_values,"o-")
            plt.plot(new_x,y_values,"o-")
            plt.title(title)
            plt.savefig(title+".png",format="png")
            plt.close()
            x_values = []
            y_values = []
            title = ""
        if line.strip().startswith("title"):
            title = line.split('"')[1]
            title = title.split()
            title = file_name+"_dim"+title[1]
            print("title "+title)

        # Split the line into words (or values)
        values = line.split()
        if title == "":
            continue
        # Check if the line contains numerical values (excluding title and set limits)
        if len(values) == 2 and all(is_float(v) for v in values):
            # Convert values to float and append to the respective lists
            x_values.append(float(values[0]))
            y_values.append(float(values[1]))

    #return x_values, y_values


def is_float(value):
    try:
        float(value)
        return True
    except ValueError:
        return False

if __name__ == '__main__':
    import sys, os, argparse

    ##############################
    #  Parse command line input  #
    ##############################
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Parse file and extract x, y values and title.")
    
    # Add argument for the input filename
    parser.add_argument('filename', type=str, help='The path to the file to be parsed.')

    # Parse the arguments
    args = parser.parse_args()


    try:
        with open(args.filename, 'r') as file:
            file_content = file.read()
            parse_file_contents(file_content,args.filename.rstrip(".top"))

    except FileNotFoundError:
        print(f"File {args.filename} not found. Please provide a valid file.")
