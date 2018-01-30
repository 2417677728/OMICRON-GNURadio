# OMICRON - GNU Radio
This repository contains the modules and examples inside the OMICRON project, realized by the TSC of the URJC.
The aim of this experiments is to try the flexibility of SDR and GNU radio, realizing different types of applications and develop time and frequency adaptive transceivers.

## Installation
### Dependencies
#### GNU Radio 3.7
The version of GNU Radio required is the 3.7.10. Some components, especially from the OFDM folder may not work correctly with previous versions.
Instructions for installing it can be found [here](https://wiki.gnuradio.org/index.php/InstallingGRFromSource).

#### gr-ieee802-11
The code of this project is based on the repostory gr-ieee802-11 which has an OFDM transceiver compatible with the 802.11 a/g/p standard. For this reason, it is necessary to install this modules and its dependencies. The instructions for doing this can be found [here](https://github.com/bastibl/gr-ieee802-11#installation).

### Installation of OMICRON-GNURadio
This steps will install the two modules that contains this repository. The explanation of this modules can be found later.
```
git clone https://github.com/reysam93/OMICRON-GNURadio.git
# installing gr-GNU_tutorials
cd gr-GNU_tutorials
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig

# installing gr-adaptiveOFDM
cd gr-adaptiveOFDM
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig

# installing gr-frequencyAdaptiveOFDM
cd gr-frequencyAdaptiveOFDM
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

## Modules
### gr-GNU_tutorials
This module contains the examples realized following the GNU Radio [guided tutorials](http://gnuradio.org/redmine/projects/gnuradio/wiki/Guided_Tutorials), allowing to run them. This tutorial offers a first contact with GNU Radio and it is highly recommended for new users.

### gr-adaptiveOFDM
This code implements the IEEE 802.11 standard using adaptive modulation and coding (AMC) in point to point transmissions. The channel is estimated at the transmitter and the modulation and coding is selected according to the desired SNR, so when the module of the channel is low the transmitter will select a more robust modulation with a reduced rate. As wireless channels are time variant, different modulations will be used during transmission, using the highest rate possible while assuring a package error rate (PER) less than a 10%.

For the channel state information at the transmitter (CSIT), a symmetric channel is assumed. Also, it is assumed that the noise power is the same in both ends of the communication.  This way, every time a new WiFi frame is receive, its SNR is estimated, and it is used to select the modulation and the redundancy of the transmission. However, this estimation will only be used during a brief time, while the channel is considering to be the same, being this time the coherent time of the channel.

In the IEEE 802.11 standard, the same modulation must be used for all OFDM symbols inside a WiFi frame, and for all subcarriers inside a symbol. Taking this into account, the SNR used for selecting the modulation and coding is the minimum SNR among all the subcarriers instead of the mean SNR for assuring a more robust encoding.

Finally, for assuring that the channel estimation is updated frequently, the transmission of ACKs has also been implemented. Any time the receiver receives a WiFi data frame, it will send an ACK message to the transmitter in BPSK, so the transmitter will be able to adapt the modulation and coding used if the channel is different.


### gr-frequencyAdaptiveOFDM
In this block, the gr-adaptiveOFDM module is modified so its becomes adaptive in frequency. As explained before, in the gr-adaptiveOFDM module the same modulation is used for every carrier. However, in wireless communication the channel is usually frequency selective due to the multiple paths existing between transmitter and receiver. This imply that the module of the channel may be different along the bandwidth used for the OFDM symbols, so a higher spectral efficiency could be achieve using different modulations for different carries.

In this module, the subcarriers of the OFDM symbols are divided in four resource blocks, and each of them may have a different modulation. The modulation of each resource block is decided using the minimum SNR among all the subcarriers belonging to this resource block. Therefore, if the channel is frequency selective, the minimum SNR of the different resource blocks will be different, and the subcarriers with higher SNR will have a more efficient modulation while subcarriers with lower SNR are using a more robust one. In addition, the different modulations used inside an OFDM symbol are repeated for all symbols inside belonging to the same WiFi frame, as the coherence time is assumed to be longer than the transmission time of one frame.

The counter part of this improvement is that it is no longer compatible with the IEEE 802.11 standard, as it uses different headers. This is necessary because while in the standard the header contains information for one MCS, this new version contains the MCS for the four different resource blocks.


## Experiments

