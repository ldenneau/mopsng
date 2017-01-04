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

db <- mopsdb('comets')
png(file=filename, width=1400, height=1400, units="px", pointsize=12, type="cairo", antialias="subpixel")

par(mar=c(5,20,2,2))
all = db$query("select orb.q q, concat(orb.mpc_desig, '/', orb.longname) name, obs.r_au r from mpc_orb orb join mpc_obs obs using(mpc_desig) order by q")
name.f = factor(all$name, levels=unique(all$name[order(all$q)]))
stripchart(all$q ~ name.f, cex.axis=.5, xlim=c(.2,50), xlab='Heliocentric Distance, AU', main='PS1 Comet Observations', log='x', pch=0, cex=.7, las=2, cex.axis=1.0, col='red')
stripchart(all$r ~ name.f, pch=0, col='black', cex=.7, add=T)

dev.off()
