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
git clone git@github.com:Notchayan/HackyDB.git
cd HackyDB
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

### 3. Run the Container
To run the project in an interactive container:
```bash
docker run -it --rm hackydb
```
This will:
- Start a temporary container.
- Run the HackyDB executable located at `./bin/HackyDb`.

## Acknowledgments
We extend our gratitude to **Prof. Arnab Bhattacharya** for his invaluable guidance and for providing us with the opportunity to deepen our understanding of database systems.

---
For any queries or discussions, feel free to reach out!
