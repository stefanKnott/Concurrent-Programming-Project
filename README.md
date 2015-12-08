# Concurrent-Programming-Project
Project for CSCI 4830 Spc Topics

This project's goal was to analyze and compare different locking mechanisms across varying systems.

Testscripts (written in BASH) are used to extract runtime data (CPU usage, total run time, context switches) from the program.
In the program 4 different locking mechanisms are used: pthread_mutex, pthread_spinlock, semaphore, and a test-test-and-set lock with exponential backoff.  These locks were tested across varying: CPU set size, thread pool size, and contention levels.
Final conclusions regarding what I define as stressed and relaxed systems can be found in the writeup pdf and presentation.
