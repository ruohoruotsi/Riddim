load SlowlyOnsettingSound.mat
plot(y)
text(250, y(250), 'a slowing onsetting sound \rightarrow', 'HorizontalAlignment', 'right')
text(351, y(351), 'peak point in the first derivative \rightarrow', 'HorizontalAlignment','right')
text(338, y(338), '\leftarrow a local minimum on the way to the peak',
'HorizontalAlignment', 'left')
