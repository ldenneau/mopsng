args <- commandArgs(trailingOnly=T)
#efffile <- args[0]
efffile <- '55886.OSS.eff'
savefile <- args[1]

# R code to fit erfc() for various data
title = paste('IPP efficiency, effile')
xlab = 'V mag'
ylab = 'Throughput (NF/NK)'

# IPP base
eff <- read.table(efffile, header=T)
effx = eff$MAG + 0.25 / 2
effy = eff$NF / eff$NK
effy[which(is.nan(effy))] = 0       # hack to fix NK==0 bins
efffit = nls(effy~eff0/(1+exp((effx-L)/w)),start=list(eff0=0.5,w=.5,L=21.0),control=list(maxiter=500),trace=TRUE)
plot(effx,effy,pch=1,col='black', main=title, xlab=xlab, ylab=ylab,ylim=c(0,1))
effpre = predict(efffit)
lines(effx,effpre,col='black')


# Limit lines
eff0 <- efffit$m$getPars()['eff0']
Leff <- efffit$m$getPars()['L']
weff <- efffit$m$getPars()['w']

abline(h=eff0,col='black',lty='dashed')
abline(v=Leff,col='black',lty='dashed')
#abline(v=Leff-weff,col='red',lty='dashed')
#abline(v=Leff+weff,col='red',lty='dashed')

# legend
text(18,0,labels=c(sprintf('eff0 = %.2f\nL = %.2f', eff0, Leff)),adj=c(0,0))

