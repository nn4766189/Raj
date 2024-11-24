# Use an official Python runtime as a parent image
FROM python:3.9-slim as builder

# Set the working directory in the container
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . /app

# Install required system packages for building C++ and Python dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Compile the C++ program
RUN g++ -o my_program loda.cpp -std=c++11 -pthread

# Stage 2: Final lightweight runtime image
FROM python:3.9-slim

# Set the working directory
WORKDIR /app

# Copy only the necessary files and compiled binary from the builder stage
COPY --from=builder /app /app

# Expose port 80 to the host
EXPOSE 80

# Define environment variable
ENV NAME World

# Run the Python script and C++ program when the container launches
CMD ["bash", "-c", "./my_program && python soul.py"]
