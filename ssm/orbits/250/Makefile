# Makefile to create insertion file for 1/250 model.  This will be a true
# scaling of the full model to 1/250 size.  Some populations will be 
# statistically dubious due to small numbers.

# Make options.
.SECONDARY :
.PHONY : all

# Our stuff.  FILES1 is Jedicke-created; FILES2 is Grav, requires conversion
# of some orbital elements; FILES3 is Grav, requires conversion and manual
# filtering of problematic comets (sungrazers).  Notes TNOs (ST) also
# require special numbering at the insertion step.
SAMPLE = 250
FILES1 = S0 S1.01 S1.02 S1.03 S1.04 S1.05 S1.06 S1.07 S1.08 S1.09 S1.10 SC 
FILES2 = St4 St5 St6 St7 St8 ST SS
FILES3 = Sc

# Master rule.
all : done_1 done_2 done_3


# Rules to generate sampled lists of orbital elements to process.
$(FILES1) :
	bunzip2 -c ../RAW/$@.bz2 | sample $(SAMPLE) > $@

$(FILES2) :
	bunzip2 -c ../RAW/$@.bz2 | sample $(SAMPLE) > $@

# Need to remove bad comets
$(FILES3) :
	bunzip2 -c ../RAW/$@.bz2 | sample $(SAMPLE) | egrep -v 'Sc004A5|Sc004cC|Sc003DG|Sc003GC|Sc003DL' > $@


# Rules to insert orbital elements into PSMOPS instance.
done_1 : S0 S1.01 S1.02 S1.03 S1.04 S1.05 S1.06 S1.07 S1.08 S1.09 S1.10 SC 
	insertSyntheticOrbits $(FILES1)
	touch done_1

done_2 : ST
	insertSyntheticOrbits --grav --b62 ST00000 ST
	touch done_2

done_3 : SS Sc St4 St5 St6 St7 St8
	insertSyntheticOrbits --grav SS Sc St4 St5 St6 St7 St8
	touch done_3
