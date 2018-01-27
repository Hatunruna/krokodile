#!/bin/sh

montage -geometry +0+0 -background none raw/*Head.png krokodile/kreature_head.png
montage -geometry +0+0 -background none raw/*Body.png krokodile/kreature_body.png
montage -geometry +0+0 -background none raw/*PosteriorLeg.png krokodile/kreature_postleg.png
montage -geometry +0+0 -background none raw/*AnteriorLeg.png krokodile/kreature_anteleg.png
montage -geometry +0+0 -background none raw/*Tail.png krokodile/kreature_tail.png
