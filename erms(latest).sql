-- phpMyAdmin SQL Dump
-- version 5.2.2
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Jan 07, 2026 at 03:57 PM
-- Server version: 8.4.3
-- PHP Version: 8.3.26

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `erms`
--

-- --------------------------------------------------------

--
-- Table structure for table `assignments`
--

CREATE TABLE `assignments` (
  `assg_id` int NOT NULL,
  `runner_id` int NOT NULL,
  `errand_id` int NOT NULL,
  `assigned_time` datetime NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `errands`
--

CREATE TABLE `errands` (
  `errand_id` int NOT NULL,
  `requester_id` int NOT NULL,
  `description` text,
  `pickup_loc` varchar(255) DEFAULT NULL,
  `dropoff_loc` varchar(255) DEFAULT NULL,
  `distance` decimal(5,2) DEFAULT NULL,
  `status` enum('Pending','Assigned','Completed') DEFAULT 'Pending',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `runner_id` int DEFAULT NULL,
  `runner_earned` tinyint(1) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `errands`
--

INSERT INTO `errands` (`errand_id`, `requester_id`, `description`, `pickup_loc`, `dropoff_loc`, `distance`, `status`, `created_at`, `updated_at`, `runner_id`, `runner_earned`) VALUES
(2, 4, 'prosperiti', 'Mekdi DT', 'Satria (Kasturi)', 1.02, 'Completed', '2025-12-01 22:37:14', '2025-12-11 09:20:45', 5, 0),
(3, 4, 'test', 'test', 'tet', 2.50, 'Completed', '2025-12-01 23:35:27', '2025-12-01 23:37:17', NULL, 0),
(6, 4, 'Mekdi', 'satria', 'dt175', 3.13, 'Assigned', '2025-12-09 01:42:34', '2025-12-09 01:44:54', 2, 0),
(7, 4, 'mekdi', 'DT', 'DTuwu', 1.03, 'Assigned', '2025-12-11 09:08:51', '2025-12-23 00:25:45', 20, 0),
(8, 4, 'Buku', 'Dt', 'DT175', 2.17, 'Completed', '2025-12-11 09:18:54', '2025-12-16 23:50:49', 9, 0),
(9, 4, 'Test', 'Test2', 'Test3', 8.40, 'Completed', '2025-12-17 00:01:16', '2025-12-17 00:29:40', 9, 0),
(10, 4, 'Buku teks', 'Satria', 'DT175', 4.61, 'Pending', '2025-12-17 02:15:07', '2025-12-17 02:15:07', NULL, 0),
(11, 4, 'Buku Teks', 'Satria', 'DT175', 4.97, 'Completed', '2025-12-17 02:18:18', '2025-12-23 23:17:34', 20, 1),
(12, 4, 'Buku Teks', 'Satria', 'DT', 7.16, 'Completed', '2025-12-17 02:37:33', '2025-12-23 23:10:43', 20, 1),
(13, 4, 'Buku', 'Satria', 'DT', 7.31, 'Pending', '2025-12-17 02:38:52', '2025-12-17 02:38:52', NULL, 0),
(14, 4, 'Buku2', 'Sat', 'DT', 9.31, 'Pending', '2025-12-17 02:56:29', '2025-12-17 02:56:29', NULL, 0),
(15, 4, 'Luu', 'lugua', 'bro', 11.07, 'Completed', '2025-12-17 03:11:59', '2025-12-23 23:10:43', 20, 1),
(16, 22, 'parcel', 'ppp', 'aj', 6.58, 'Completed', '2025-12-23 00:41:41', '2025-12-23 23:03:55', 20, 1),
(17, 26, 'Untuk ApitRunner', 'Satria', 'DT', 8.37, 'Completed', '2025-12-23 23:15:18', '2025-12-23 23:17:05', 20, 1),
(18, 26, 'Parcel Dari PPP', 'PPP', 'Satira', 13.62, 'Completed', '2026-01-07 23:35:23', '2026-01-07 23:38:42', 20, 1);

-- --------------------------------------------------------

--
-- Table structure for table `payments`
--

CREATE TABLE `payments` (
  `payment_id` int NOT NULL,
  `quote_id` int NOT NULL,
  `errand_id` int NOT NULL,
  `price` decimal(6,2) NOT NULL,
  `pay_status` enum('Paid','Pending','Failed') DEFAULT 'Pending',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `payment_method` enum('CreditCard','OnlineBanking','EWallet') DEFAULT 'CreditCard',
  `transaction_id` varchar(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `payments`
--

INSERT INTO `payments` (`payment_id`, `quote_id`, `errand_id`, `price`, `pay_status`, `created_at`, `updated_at`, `payment_method`, `transaction_id`) VALUES
(1, 2, 14, 27.93, 'Paid', '2025-12-17 02:59:02', '2025-12-17 03:10:38', 'CreditCard', '1'),
(2, 2, 14, 27.93, 'Paid', '2025-12-17 03:00:16', '2025-12-17 03:11:07', 'CreditCard', '2'),
(3, 2, 14, 27.93, 'Paid', '2025-12-17 03:01:36', '2025-12-17 03:11:20', 'CreditCard', '3'),
(4, 3, 15, 33.21, 'Paid', '2025-12-17 03:19:30', '2025-12-17 03:19:30', 'CreditCard', 'ERMS#118843'),
(5, 4, 16, 19.74, 'Paid', '2025-12-23 00:43:53', '2025-12-23 00:43:53', 'CreditCard', 'ERMS#110060'),
(6, 5, 17, 25.11, 'Paid', '2025-12-23 23:15:57', '2025-12-23 23:15:57', 'CreditCard', 'ERMS#112833'),
(7, 6, 18, 40.86, 'Paid', '2026-01-07 23:36:23', '2026-01-07 23:36:23', 'CreditCard', 'ERMS#121955');

-- --------------------------------------------------------

--
-- Table structure for table `quotations`
--

CREATE TABLE `quotations` (
  `quote_id` int NOT NULL,
  `errand_id` int NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `base_price_per_km` decimal(10,2) NOT NULL DEFAULT '1.00',
  `distance_km` decimal(10,2) NOT NULL DEFAULT '0.00',
  `runner_percentage` decimal(5,2) NOT NULL DEFAULT '70.00',
  `system_percentage` decimal(5,2) NOT NULL DEFAULT '30.00',
  `runner_share` decimal(10,2) DEFAULT '0.00',
  `system_fee` decimal(10,2) DEFAULT '0.00',
  `status` enum('Pending','Paid') DEFAULT 'Pending',
  `transaction_id` varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `quotations`
--

INSERT INTO `quotations` (`quote_id`, `errand_id`, `created_at`, `updated_at`, `base_price_per_km`, `distance_km`, `runner_percentage`, `system_percentage`, `runner_share`, `system_fee`, `status`, `transaction_id`) VALUES
(1, 13, '2025-12-17 02:38:52', '2025-12-17 02:39:17', 3.00, 7.31, 70.00, 30.00, 15.35, 6.58, 'Paid', 'ERMS#10963'),
(2, 14, '2025-12-17 02:56:29', '2025-12-17 03:01:36', 3.00, 9.31, 70.00, 30.00, 19.55, 8.38, 'Paid', 'ERMS#115335'),
(3, 15, '2025-12-17 03:11:59', '2025-12-17 03:19:30', 3.00, 11.07, 70.00, 30.00, 23.25, 9.96, 'Paid', 'ERMS#118843'),
(4, 16, '2025-12-23 00:41:41', '2025-12-23 00:43:53', 3.00, 6.58, 70.00, 30.00, 13.82, 5.92, 'Paid', 'ERMS#110060'),
(5, 17, '2025-12-23 23:15:18', '2025-12-23 23:15:57', 3.00, 8.37, 70.00, 30.00, 17.58, 7.53, 'Paid', 'ERMS#112833'),
(6, 18, '2026-01-07 23:35:23', '2026-01-07 23:36:23', 3.00, 13.62, 70.00, 30.00, 28.60, 12.26, 'Paid', 'ERMS#121955');

-- --------------------------------------------------------

--
-- Table structure for table `reports`
--

CREATE TABLE `reports` (
  `report_id` int NOT NULL,
  `total_errands` int DEFAULT NULL,
  `top_runner` varchar(255) DEFAULT NULL,
  `report_date` date DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `system_revenue` decimal(10,2) DEFAULT NULL,
  `total_runner_revenue` decimal(10,2) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `reports`
--

INSERT INTO `reports` (`report_id`, `total_errands`, `top_runner`, `report_date`, `created_at`, `updated_at`, `system_revenue`, `total_runner_revenue`) VALUES
(1, 14, 'apitrunner', '2026-01-07', '2026-01-07 21:26:39', '2026-01-07 21:26:39', 38.37, 89.55),
(2, 14, 'apitrunner', '2026-01-07', '2026-01-07 21:40:37', '2026-01-07 21:40:37', 38.37, 89.55),
(3, 14, 'apitrunner', '2026-01-07', '2026-01-07 21:41:57', '2026-01-07 21:41:57', 38.37, 89.55),
(4, 14, 'apitrunner', '2026-01-07', '2026-01-07 21:48:36', '2026-01-07 21:48:36', 38.37, 89.55),
(5, 15, 'apitrunner', '2026-01-07', '2026-01-07 23:39:22', '2026-01-07 23:39:22', 50.63, 118.15);

-- --------------------------------------------------------

--
-- Table structure for table `runners`
--

CREATE TABLE `runners` (
  `runner_id` int NOT NULL,
  `avail_status` enum('On Duty','Off Duty') NOT NULL DEFAULT 'Off Duty',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `user_id` int NOT NULL,
  `name` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `c_number` varchar(15) DEFAULT NULL,
  `role` int NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`user_id`, `name`, `email`, `password`, `c_number`, `role`, `created_at`, `updated_at`) VALUES
(1, 'Afiq', 'apitpro@gmail.com', 'Afiqmuhaimin1', '01123427786', 1, '2025-12-01 21:45:20', '2025-12-01 21:45:20'),
(2, 'buburamen', 'bubu@gmail.com', 'f438248bb83506fc2572ae3820b87aa2305205cd39394ddaf5451aba6dc05978', '9112', 1, '2025-12-01 22:03:36', '2025-12-01 22:03:36'),
(3, 'bub', 'bub', 'b4f309f20d7b965370de1ae4a4b4264e8cc16c7e640bdee921ae52565bd35b25', '123', 1, '2025-12-01 22:05:02', '2025-12-01 22:05:02'),
(4, 'apit', 'apit@gmail.com', 'dc649a9ccf04f68f1f4abc4639569de983e4204b733eafd5938eed1b0eab4b20', '01162674383', 0, '2025-12-01 22:17:18', '2025-12-01 22:17:18'),
(5, 'bob', 'apitpro123@gmail.com', '488543822b2411dbd12837441e0e16a47979f00ae1193df4b12505c051c8f573', '01233478832', 1, '2025-12-04 11:45:10', '2025-12-04 11:45:10'),
(7, 'bob', 'bob@imail.com', '8d059c3640b97180dd2ee453e20d34ab0cb0f2eccbe87d01915a8e578a202b11', '01197487643', 0, '2025-12-04 12:05:38', '2025-12-04 12:05:38'),
(8, 'admin', 'admin@gmail.com', '3eb3fe66b31e3b4d10fa70b5cad49c7112294af6ae4e476a1c405155d45aa121', '0113245564', 1, '2025-12-09 00:03:46', '2025-12-09 00:03:46'),
(9, 'runner', 'runner@gmail.com', '84b26ac683fc6760b2cec3bb0c6e2170c20e2636cfa41f1475694addd3abbcdd', '01128950075', 2, '2025-12-09 00:11:41', '2025-12-09 00:11:41'),
(10, 'userone', 'user1@gmail.com', '651f2405cf166b1dd8fdeb63e7660a9b4fb6eec380cb7df5436951d08703d3d5', '01187586677', 0, '2025-12-09 00:38:32', '2025-12-09 00:38:32'),
(11, 'adminone', 'admin1@gmail.com', '73da228abd9ea6a477e414af02cf912ee046a8832e6f34b424e37d626769627e', '01238519521', 1, '2025-12-09 00:39:26', '2025-12-09 00:39:26'),
(12, 'runnerone', 'runner1@gmail.com', '89b12a27c1fab4b88a2bde18b934b1987e5748daf2c533a613b6f77226801af3', '01188489982', 2, '2025-12-09 00:40:20', '2025-12-09 00:40:20'),
(14, 'Runnerr', 'uwu@gmail.com', 'df3d25500909418d33f2a6d2efd2e1e5a8e8c8df94c0191fb2df2ff027a1724b', '01162678454', 2, '2025-12-09 01:46:53', '2025-12-09 01:46:53'),
(15, 'ApitBob', 'apit@sadasd.com', 'e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7', '0178163798', 1, '2025-12-11 09:00:24', '2025-12-11 09:00:24'),
(16, 'byb hh', 'a@zyman.com', 'b0ee36febe5d3df14a0f9d275ceab7b8165c5ccd1dca8f84b6e7fa80388ada8c', '9019992928', 0, '2025-12-11 09:16:49', '2025-12-11 09:16:49'),
(17, 'entahlanak', 'entah@e.com', 'b20c99df8b2477408bb3b1d9e31889391e5dfee5544e039b4023737e47f76455', '01162674383', 0, '2025-12-17 00:08:20', '2025-12-17 00:08:20'),
(18, 'John', 'john@gmail.com', '20d8ff2150b414bcbf716d4b1954422a9dcc1b90ac1f6f1951ddc5dc62586f34', '+16758774321', 0, '2025-12-17 00:13:29', '2025-12-17 00:13:29'),
(19, 'ApitMentari', 'apitmentari@mentari.com', 'f4dab3d7cbd3e4ea', '0128946673', 0, '2025-12-23 00:17:47', '2025-12-23 00:17:47'),
(20, 'apitrunner', 'apitrunner@gmail.com', 'f4dab3d7cbd3e4ea', '0125468872', 2, '2025-12-23 00:25:14', '2026-01-07 22:29:11'),
(21, 'hakim ', 'zona@gmail.com', '3ddbee10828763ad', '60145808624123', 0, '2025-12-23 00:36:45', '2025-12-23 00:36:45'),
(22, 'hakim', 'hakim@gmail.com', 'b02eebb0f6e3c4fd', '0127786645', 0, '2025-12-23 00:40:34', '2025-12-23 00:40:34'),
(23, 'ApitAdmin', 'apit@admin.com', 'df95b0d31160a8d9', '0123895532', 1, '2025-12-23 00:49:50', '2025-12-23 00:49:50'),
(24, 'soli', 'soli@joli.com', 'd90567e52a18772a', '0146324516', 2, '2025-12-23 00:54:48', '2025-12-23 00:54:48'),
(25, 'AdminSatu', 'adminsatu@erms.com', 'd2467995be2e3f8f', '0127513247', 1, '2025-12-23 23:08:18', '2025-12-23 23:08:18'),
(26, 'UserSatu', 'usersatu@erms.com', 'b770cf2ab9bedcfd', '0128590032', 0, '2025-12-23 23:14:18', '2025-12-23 23:14:18'),
(27, '1547886', 'uwu@pep.com', 'f4dab3d7cbd3e4ea', '01116879969', 0, '2026-01-07 22:24:21', '2026-01-07 22:24:21');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `assignments`
--
ALTER TABLE `assignments`
  ADD PRIMARY KEY (`assg_id`),
  ADD KEY `runner_id` (`runner_id`),
  ADD KEY `errand_id` (`errand_id`);

--
-- Indexes for table `errands`
--
ALTER TABLE `errands`
  ADD PRIMARY KEY (`errand_id`),
  ADD KEY `requester_id` (`requester_id`),
  ADD KEY `fk_runner` (`runner_id`);

--
-- Indexes for table `payments`
--
ALTER TABLE `payments`
  ADD PRIMARY KEY (`payment_id`),
  ADD KEY `fk_payment_quote` (`quote_id`),
  ADD KEY `fk_payment_errand` (`errand_id`);

--
-- Indexes for table `quotations`
--
ALTER TABLE `quotations`
  ADD PRIMARY KEY (`quote_id`),
  ADD KEY `errand_id` (`errand_id`);

--
-- Indexes for table `reports`
--
ALTER TABLE `reports`
  ADD PRIMARY KEY (`report_id`);

--
-- Indexes for table `runners`
--
ALTER TABLE `runners`
  ADD PRIMARY KEY (`runner_id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`user_id`),
  ADD UNIQUE KEY `email` (`email`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `assignments`
--
ALTER TABLE `assignments`
  MODIFY `assg_id` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `errands`
--
ALTER TABLE `errands`
  MODIFY `errand_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=19;

--
-- AUTO_INCREMENT for table `payments`
--
ALTER TABLE `payments`
  MODIFY `payment_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;

--
-- AUTO_INCREMENT for table `quotations`
--
ALTER TABLE `quotations`
  MODIFY `quote_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT for table `reports`
--
ALTER TABLE `reports`
  MODIFY `report_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `user_id` int NOT NULL AUTO_INCREMENT;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `assignments`
--
ALTER TABLE `assignments`
  ADD CONSTRAINT `assignments_ibfk_1` FOREIGN KEY (`runner_id`) REFERENCES `runners` (`runner_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `assignments_ibfk_2` FOREIGN KEY (`errand_id`) REFERENCES `errands` (`errand_id`) ON DELETE CASCADE;

--
-- Constraints for table `errands`
--
ALTER TABLE `errands`
  ADD CONSTRAINT `errands_ibfk_1` FOREIGN KEY (`requester_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `fk_runner` FOREIGN KEY (`runner_id`) REFERENCES `users` (`user_id`);

--
-- Constraints for table `payments`
--
ALTER TABLE `payments`
  ADD CONSTRAINT `fk_payment_errand` FOREIGN KEY (`errand_id`) REFERENCES `errands` (`errand_id`),
  ADD CONSTRAINT `fk_payment_quote` FOREIGN KEY (`quote_id`) REFERENCES `quotations` (`quote_id`),
  ADD CONSTRAINT `payments_ibfk_1` FOREIGN KEY (`quote_id`) REFERENCES `quotations` (`quote_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `payments_ibfk_2` FOREIGN KEY (`errand_id`) REFERENCES `errands` (`errand_id`) ON DELETE CASCADE;

--
-- Constraints for table `quotations`
--
ALTER TABLE `quotations`
  ADD CONSTRAINT `quotations_ibfk_1` FOREIGN KEY (`errand_id`) REFERENCES `errands` (`errand_id`) ON DELETE CASCADE;

--
-- Constraints for table `runners`
--
ALTER TABLE `runners`
  ADD CONSTRAINT `fk_runner_user` FOREIGN KEY (`runner_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `runners_ibfk_1` FOREIGN KEY (`runner_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
