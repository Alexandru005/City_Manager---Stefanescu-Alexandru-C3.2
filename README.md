# City_Manager

## Student Information
* **Name:** Stefanescu Alexandru Stefan
* **University:** Politehnica University of Timisoara
* **Year:** 2
* **Group:** C3.2

## Project Description
`city_manager` is a UNIX-based Command Line Interface (CLI) application written in C, designed to manage and monitor urban infrastructure reports (potholes, broken lighting, flooding, etc.) across different city districts.

## Phase 1
* **File Management:** Create and manipulate binary (`reports.dat`) and text files (`district.cfg`, `logged_district`) per district using exclusively system calls (`open`, `read`, `write`, `lseek`, `ftruncate`).
* **Access Control:** Implement `manager` and `inspector` roles, passed as command-line arguments.
* **UNIX Permissions:** Programmatically set (`chmod`) and strictly validate (`stat`) file permissions (e.g., 750, 664, 640) before any operation to ensure role-based security.
* **Core Commands:** Develop the core functionalities: `add`, `list`, `view`, `remove_report` (manager only), `update_threshold` (manager only), and `filter`.
* **Symbolic Links:** Create and manage symlinks (`active_reports-<district_id>`) pointing to the report files, using `lstat()` to correctly identify them.
* **AI Integration:** Use an AI assistant **strictly** to generate the `parse_condition` and `match_condition` helper functions for the `filter` command. Full documentation of prompts and manual corrections must be logged in the `ai_usage.md` file.