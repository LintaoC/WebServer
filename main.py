import sys
from PIL import Image

def copy_image(description, filename):
    try:
        # Load the existing image
        img = Image.open('../b.png')
        
        # Save the image with the new filename
        img.save(filename + '.png')
        return filename + '.png'
    except Exception as e:
        return str(e)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <description> <filename>")
    else:
        description = sys.argv[1]
        filename = sys.argv[2]
        result = copy_image(description, filename)
        print(f"Image saved as {result}")

