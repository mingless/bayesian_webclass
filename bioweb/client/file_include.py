## @file file_include.py
#  @brief helping python script for substitution files when generationg. Used in scons scripts.

import re, fileinput, sys, os


def file_substitution(in_file, out_file, inc_dir):
    """function to include html into server side. Look for <!-- #include "FileName" --> statements and include the files"""

    print 'file_substitution, in_file:' + str(in_file) + ', out_file:' + str(out_file) + ', inc_dir:' + str(inc_dir)
    try:
        f = open(in_file, "r")
        w = open(out_file, "w")

        for line in f:
            m = re.search(r'<!--\s*#include\s*"(.*)"\s*-->', line)
            if m:
                inc_name = os.path.join(inc_dir, m.group(1) )
                print 'file_include.py, include file ' + str(inc_name) + ' into ' + str(out_file)

                with open(inc_name) as inc:
                    text = inc.read()
                    #print 'include text ' + text
                    w.write(text) #include the file
            else:
                w.write(line)
    except:
        pass




