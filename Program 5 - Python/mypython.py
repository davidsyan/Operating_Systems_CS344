#!/usr/bin/python

import string
import random

def random_number():
	return random.randint(1,42)

def random_letter():
	return ''.join(random.choice(string.ascii_lowercase) for x in range(10))

str1 = random_letter()
str2 = random_letter()
str3 = random_letter()

print("Random Letters")

file1 = open("mypython_file1", "w")
file1.write(str1)
print(str1)
file1.close()

file2 = open("mypython_file2", "w")
file2.write(str2)
print(str2)
file2.close()

file3 = open("mypython_file3", "w")
file3.write(str3)
print(str3)
file3.close()

print("\nRandom Numbers")
num1 = random_number()
print(num1)
num2 = random_number()
print(num2)

print("\nNumbers Multiplied")
product = num1 * num2
print(product)
