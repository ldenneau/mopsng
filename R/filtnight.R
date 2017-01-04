#!/usr/bin/env R
# Usage: filtnight --args dbname nightnum
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
dbname <- args[1]
nn <- as.integer(args[2])
filter_id = args[3]
filename <- args[4]
db <- mopsdb(dbname)

colmap = list(r='red', g='darkgreen', i='darkorange1', y='goldenrod', z='steelblue', w='grey')
col = colmap[[filter_id]]
xlab = filter_id
ylab = 'S/N'
title = paste(dbname, ' ', filter_id, sep='')

q = sprintf('select d.s2n s2n, d.mag mag from detections d join tracklet_attrib ta using(det_id) join tracklets t using(tracklet_id) join fields f on t.field_id=f.field_id where t.classification="C" and f.nn=%d and d.filter_id="%s"', nn, filter_id)
print(q)
#Sr = db$query('select d.s2n s2n, d.mag mag from detections d join tracklet_attrib ta using(det_id) join tracklets t using(tracklet_id) join fields f on t.field_id=f.field_id where t.classification="C" and f.nn=56336 and d.filter_id="r"')
Sr = db$query(q)

q = sprintf('select d.s2n s2n, d.mag mag from detections d join tracklet_attrib ta using(det_id) join tracklets t using(tracklet_id) join fields f on t.field_id=f.field_id where t.classification="N" and f.nn=%d and d.filter_id="%s"', nn, filter_id)
print(q)
#Nr = db$query('select d.s2n s2n, d.mag mag from detections d join tracklet_attrib ta using(det_id) join tracklets t using(tracklet_id) join fields f on t.field_id=f.field_id where t.classification="N" and f.nn=56336 and d.filter_id="r"')
Nr = db$query(q)

png(file=filename, width=800, height=600, units="px", pointsize=12, type="cairo", antialias="subpixel")

if (length(Nr) == 0) {
    plot(Sr$mag, Sr$s2n, log='y', pch=20, col=col, cex=.75, xlim=c(15,25), ylim=c(3,500), main=title, xlab=xlab, ylab=ylab)
    legend('topright', c('Synthetic'), col=c(col), pch=c(15))
} else {
    plot(Nr$mag, Nr$s2n, log='y', pch=20, col='black', cex=.4, xlim=c(15,25), ylim=c(3,500), main=title, xlab=xlab, ylab=ylab)
    points(Sr$mag, Sr$s2n, pch=20, col=col, cex=.75)
    legend('topright', c('Synthetic', 'Nonsynthetic'), col=c(col, 'black'), pch=c(15, 15))
}

dev.off()

