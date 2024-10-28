# -*- coding: utf-8 -*-
"""
Created on Sat Oct 28 2022

@author: Bence Mány

DTU Master thesis: Algorithm development for ultrasound-based robotic artery detection 
(RAPID project)
"""

import imaging_m as m
import os
import time


def test_m_mode():
    
    path_pump = "data\M-mode\phantom01_pump"
    path_no_pump = "data\M-mode\phantom01_no_pump"
    path_no_vessel = "data\M-mode\phantom01_no_vessel"
    path_random = "data\M-mode\phantom01_random"
    path_eval = "data\M-mode\evaluation"
    
    start = time.time()
    
    lines = []
    #Pump data
    for x in os.listdir(path_pump):
        if x.endswith(".csv"):
            path = path_pump + '\\' + x 
            print("\n\nFile: ", path)
            result, amp, period = m.m_mode(path, True)
            lines.append([x, 1, result, amp, period])
    #No pump data
    # for x in os.listdir(path_no_pump):
    #     if x.endswith(".csv"):
    #         path = path_no_pump + '\\' + x 
    #         print("\n\nFile: ", path)
    #         result, amp, period = m.m_mode(path)
    #         lines.append([x, 0, result, amp, period])
    # #No vessel data
    # for x in os.listdir(path_no_vessel):
    #     if x.endswith(".csv"):
    #         path = path_no_vessel + '\\' + x 
    #         print("\n\nFile: ", path)
    #         result, amp, period = m.m_mode(path)
    #         lines.append([x, 0, result, amp, period])
    # #Random data
    # for x in os.listdir(path_random):
    #     if x.endswith(".csv"):
    #         path = path_random + '\\' + x 
    #         print("\n\nFile: ", path)
    #         result, amp, period = m.m_mode(path)
    #         lines.append([x, 0, result, amp, period])
            
    end = time.time()
    print("Average execution time = ", (end-start) / len(lines), "s")
    
    m.save_results(path_eval, lines)

if __name__ == '__main__':    


    # #Testing and evaluating the B-mode detection algorithm
    # test_b_mode()
    # b.evaluation()
    
    #Testing and evaluating the M-mode detection algorithm
    test_m_mode()
    m.evaluation()
    
