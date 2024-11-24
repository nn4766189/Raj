FROM python:3.9-slim

WORKDIR /app

COPY . .


RUN chmod +x *


RUN pip install -r requirements.txt


CMD ["python", "raja.py"]
