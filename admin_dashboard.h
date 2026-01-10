#ifndef ADMIN_DASHBOARD_H
#define ADMIN_DASHBOARD_H

#include <string>

// ===== User Management =====
void viewAllUsers();               // View all users (including role info)
void registerRunner();             // Admin can manually register a runner

// ===== Errand Management =====
void manageErrands();              // Admin can assign errands, update statuses, cancel, etc.

// ===== Reporting =====
void generateReports();            // Generate reports (e.g., summary stats, earnings, errands completed)
void showMonthlyErrandsChart();    // Display monthly errands bar chart

// ===== Admin Menu =====
void admin_menu(const std::string& adminUsername); // Display admin dashboard menu

#endif // ADMIN_DASHBOARD_H
