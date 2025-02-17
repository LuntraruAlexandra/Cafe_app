# Cafe Management System

## Overview

This is a C++ console-based application designed to manage cafe operations efficiently across multiple cities. It handles employee management, product inventory, customer orders, events, and financial reports using various design patterns such as Singleton, Chain of Responsibility, Facade, and Observer.

## Features

- Multi-city Support: Manages cafes in multiple locations.

- Employee Management: Add, delete, and view employees.

- Product Inventory: Add new products, update stock levels, and track supplies.

- Customer Order Handling: Manages orders through a chain of responsibility involving waiters and baristas.

- Event Management: Organizes special events with unique products.

- Financial Reports: Tracks profits, expenses, and generates daily reports.

- Language Support: English and Romanian translations for menus and interactions.

## Technologies Used

- C++ STL: Utilizes file handling, threading, and smart pointers.

- File System Operations: Reads and writes CSV files for data persistence.

- Design Patterns: Implements Singleton, Chain of Responsibility, Facade, and Observer patterns.

## Installation & Setup

1. Compile the program:

``` bash
cd cafe_app/App
```

``` bash
make
```

Run the program:
``` bash
make run [Data_Folder]
```

If no folder is specified, the default "Data" folder is used.

## Usage

### 1. Main Menu

- Customer Mode:

    - Check menu

    - Place orders

    - View special event products

- Manager Mode:

    - Manage employees and products

    - Add new recipes

    - Handle event planning

    - Generate financial reports

### 2. Employee Management

- List all employees

- Add or remove employees

- Verify employee shift coverage

### 3. Product Management

- View stock levels

- Add or refill product supply

- Categorize products as ingredients or menu items

### 4. Order Processing

- Customers can order from the menu

- Orders are assigned to available waiters and baristas

- Discounts applied based on order history

### 5. Events

- Manage cafe events

- Add special event products

- Customers can place event-related orders

## File Structure

├── Data/
│   ├── București/
│   │   ├── employees.csv
│   │   ├── products.csv
│   │   ├── orders.csv
│   │   ├── recipes.csv
│   │   ├── report.csv
│   │   ├── events.csv
│   ├── Cluj-Napoca/
│   ├── ... (Other cities)
│
├── app.cpp
├── translate.txt
├── README.md

## Future Enhancements

- Add a graphical user interface (GUI)

- Implement a database for better scalability

- Integrate an API for online orders

## License

This project is open-source under the MIT License.