#!/usr/bin/env R
# Usage: defeffnight.R --args dbname nightnum
library('DBI')
library('RMySQL')

# Function to return a database handle to a MOPS database.
mopsdb <- function(dbname, host='mops01') {
    drv <- dbDriver("MySQL")
    dbi <- dbConnect(drv, user="mops", password="mops", dbname=dbname, host=host)

    list (
        query = function(query_str) {
            res <- dbSendQuery(dbi, query_str)
            fetch(res, n=-1)
        }
    )
}

# get command line args
args <- commandArgs(trailingOnly=TRUE)
filename <- args[1]

# Given a night number, compute detection efficiency for all filters observed.
#library(mopsdb)
db <- mopsdb('export')
objs = db$query("select floor((min(epoch_mjd) - 51564)/29.53059) ocnum, if(survey_mode like '%3PI%','3PI',if(survey_mode like '%SS%','SS','OTHER')) sm from export.mpc_sub m where disposition in ('C', 'A', 'R') group by desig");
ct <- table(objs$sm, objs$ocnum)

png(file=filename, width=600, height=450, units="px", pointsize=12, type="cairo", antialias="subpixel")
barplot(ct, main="PS1 NEO Discoveries by OC", xlab="OC number", ylab="Number", col=terrain.colors(3))
legend(1,30,rownames(ct),col=terrain.colors(3),pch=15)

# Cleanup
dev.off()
