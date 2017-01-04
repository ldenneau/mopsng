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
dbname <- args[1]
nn <- as.integer(args[2])
eff_type <- args[3]
filename <- args[4]

# Given a night number, compute detection efficiency for all filters observed.
#library(mopsdb)
db <- mopsdb(dbname)
filts.df <- db$query(paste('select distinct filter_id from deteff where nn=', as.character(nn)))

# For each filter, compute its efficiency.
new = TRUE
#weight = TRUE
weight = TRUE
trace = FALSE
colormap = list(g="darkgreen",r="darkred",i="darkorange1",y="goldenrod",z="steelblue",w="black")

pch = 1
legendc = c()
legendcol = c()
legendpch = c()
xlab = 'V magnitude'
ylab = 'Efficiency'
title = paste(dbname, '\nNight ', as.character(nn), ' Detection Efficiency', sep='')
png(file=filename, width=800, height=600, units="px", pointsize=12, type="cairo", antialias="subpixel")
#png(file=filename, width=800, height=600, units="px", pointsize=12)
#if (length(filts.df$names > 0)) {
    for (f in filts.df[[1]]) {
        sql <- paste("select (bin_start + bin_end) / 2 MAG, sum(found) NF, sum(known) NK from deteff where eff_type='", as.character(eff_type), "' and nn=", as.character(nn), " and filter_id='", f, "' group by bin_start having NK > 0", sep="")
        print(sql)
        eff <- db$query(sql)
        print(eff)

        effx = eff$MAG
        effy = eff$NF / eff$NK
        sumnk = sum(eff$NK)
    #    effy[which(is.nan(effy))] = 0       # hack to fix NK==0 bins
            print(f)
            print(sumnk)
        if (sumnk < 500 && f != 'w') {
            print("got sumnk < 500")
            next
        }

        print(effy)

        if (weight) {
            efffit = nls(effy~eff0/(1+exp((effx-L)/w)),start=list(eff0=0.5,w=.5,L=21.0),control=list(maxiter=500),trace=trace,algorithm='port',upper=c(1.0,5.0,40.0),lower=c(0.0,0.01,0.0),weights=eff$NK)
        } else {
            efffit = nls(effy~eff0/(1+exp((effx-L)/w)),start=list(eff0=0.5,w=.5,L=21.0),control=list(maxiter=500),trace=trace,algorithm='port',upper=c(1.0,5.0,40.0),lower=c(0.0,0.01,0.0))
        }

        # Plot
        col = colormap[[f]]
        if (new) {
            plot(effx,effy,pch=1,col=col, main=title, xlab=xlab, ylab=ylab, xlim=c(15, 25), ylim=c(0,1))
            new = FALSE
        }
        else {
            points(effx, effy, pch=pch, col=col, ylim=c(0,1))
        }
        effpre = predict(efffit)
        lines(effx,effpre,col=col)

        # Limit lines
        eff0 <- efffit$m$getPars()['eff0']
        Leff <- efffit$m$getPars()['L']
        weff <- efffit$m$getPars()['w']
        abline(v=Leff,col=col,lty='dashed')

        # Rotate colors, PCH.
        legendc[pch] = paste(f, sprintf("[eff0=%.2f,L=%.2f,n=%d]", eff0, Leff, sumnk))
        legendpch[pch] = pch
        legendcol[pch] = col
        pch = pch + 1
    }

    # Legend
    legend(21, 1.0, legendc, col=legendcol, pch=legendpch)

    # Cleanup
    dev.off()
#}
