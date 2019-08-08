# plot frame

set multiplot layout 2,3

acc_data="< tail -n 1000 log/acc.csv"
gyro_data="< tail -n 1000 log/gyro.csv"

set yrange[-5:5]
plot acc_data u 1:2 w l title "acc.x"
plot acc_data u 1:3 w l title "acc.y"
plot acc_data u 1:4 w l title "acc.z"

set yrange[-1000:1000]
plot gyro_data u 1:2 w l title "gyro.x"
plot gyro_data u 1:3 w l title "gyro.y"
plot gyro_data u 1:4 w l title "gyro.z"
