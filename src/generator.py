import sys
import openai
import json
import os
import requests

def generate_photo(description, filename):
    client = openai.OpenAI(api_key='Key')

    try:
        response = client.images.generate(
            model="dall-e-3",
            prompt=description,
            size="1024x1024",
            quality="standard",
            n=1,
        )
        image_url = response.data[0].url

        # Ensure the directory exists
        output_dir = os.getenv("ENVIRONMENT") == "remote" and "/usr/src/project/src/photos" or "../photos"
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # Download the image
        image_response = requests.get(image_url)
        if image_response.status_code == 200:
            file_path = os.path.join(output_dir, f"{filename}.png")
            with open(file_path, 'wb') as f:
                f.write(image_response.content)
            return {"image_url": image_url, "file_path": file_path}
        else:
            return {"error": f"Failed to download image, status code: {image_response.status_code}"}
    except Exception as e:
        return {"error": str(e)}

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(json.dumps({"error": "Description and filename are required"}))
    else:
        description = sys.argv[1]
        filename = sys.argv[2]
        result = generate_photo(description, filename)
        print(json.dumps(result))
