# HackyDB

## Overview
HackyDB is a database project developed as part of the **CS315 - DATABASES** course during our 6th semester at **IIT Kanpur**. This project was guided by **Prof. Arnab Bhattacharya** ([Faculty Page](https://www.cse.iitk.ac.in/users/arnabb/)).

## Team Members
HackyDB was built collaboratively by a team of five dedicated students:
- **H** - Harshit
- **A** - Aditya Gupta
- **C** - Chayan Kumawat
- **K** - Kushagra Singh
- **Y** - Yash Pratap Singh

## Project Highlights
- Designed and implemented a lightweight, efficient, and modular database system.
- Developed core functionalities such as query execution, indexing, and transaction management.
- Focused on optimizing performance and ensuring data integrity.
- Explored cutting-edge database techniques and best practices.

## 🐳 Running with Docker

### 1. Clone the Repository
```bash
https://home.iitk.ac.in/~chayank22/HackyDB.zip
download the zip file from the above code
then UnZip it
```

### 2. Build the Docker Image
Make sure Docker is installed on your system. Then run:
```bash
docker build -t hackydb .
```
This will:
- Use Ubuntu 22.04 as the base image.
- Install necessary development tools and libraries.
- Build the HackyDB binary using `make`.

### 3. Access the container 
To run the project in an interactive container:
```bash
docker run -it hackydb /bin/bash
```

### 4. Build the HackyDB
```bash
make
```

### 5. Run the HackyDB
```bash
./bin/HackyDb
```

## Testing
To test HackyDB, follow the instructions outlined in the [Link](./Test.md) file.

## Acknowledgments
We extend our gratitude to **Prof. Arnab Bhattacharya** for his invaluable guidance and for providing us with the opportunity to deepen our understanding of database systems.
