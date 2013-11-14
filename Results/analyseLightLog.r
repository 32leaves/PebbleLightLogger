library(sqldf)
library(ggplot2)

data <- sqldf("SELECT * FROM logdata", dbname = "LightLoggerDB")

# map tags
data <- transform(data, tag   = ifelse(tag == 51966, "backlight", "user"))
data <- transform(data, value = ifelse(tag == "user", 
	ifelse(value == 1, "1_TP", "2_FN"), "0_FP"))

# convert timestamps to POSIX time
data$eventTime <- as.POSIXct(data$eventTime, origin="1970-01-01")

# remove true positives
truePositiveDuplicateIndicies <- which(data$tag == "user" & data$value == "1_TP") - 1
data <- data[-truePositiveDuplicateIndicies, ]

# add dummy column for frequency plot
data <- cbind(data, dummy=rep(1,nrow(data )))

p = ggplot(data, aes(x=eventTime, y=dummy, colour=value, ymin=0, ymax=1)) +
 geom_linerange(size=.01,width=.01) +
 xlab("time") + ylab("activation") +
 ggtitle("True and false backlight activations over time") +
 geom_segment(aes(x=data$eventTime[247], xend=data$eventTime[430], y=0, yend=0), colour='black', size=1, alpha=0.3) +
 geom_segment(aes(x=data$eventTime[780], xend=data$eventTime[1309], y=0, yend=0, colour='Sports'), colour='black', size=1)
ggsave(p, filename="activations.pdf", width=20, height=2.5)
ggsave(p, filename="activations.png", width=20, height=2.5)
p


fp <- length(which(data$tag == "backlight"))
fn <- length(which(data$tag == "user" & data$value == "2_FN"))
tp <- length(which(data$tag == "user" & data$value == "1_TP"))

# fpr is not available as we don't have "true negative count"
recall <- tp / (tp + fn)
precision <- tp / (tp + fp)
F1 <- 2 * (precision * recall) / (precision + recall)

recall
precision
F1