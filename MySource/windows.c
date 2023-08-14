/*
 * windowing.c
 *
 *  Created on: May 7, 2023
 *      Author: Evgeniy
 */

#include "analyzer.h"
#include "windows.h"
#include <stdio.h>
#include <math.h>

//const float hammingTableFloat[512] = {//blackman-hurris
//	6e-05,
//6.05336e-05, 6.21352e-05, 6.48063e-05, 6.85501e-05, 7.33705e-05, 7.92729e-05, 8.62637e-05, 9.43507e-05, 0.000103543, 0.000113849,
//0.000125282, 0.000137853, 0.000151576, 0.000166466, 0.000182538, 0.00019981, 0.0002183, 0.000238027, 0.000259011, 0.000281275,
//0.000304841, 0.000329734, 0.000355978, 0.000383601, 0.00041263, 0.000443093, 0.00047502, 0.000508444, 0.000543395, 0.000579908,
//0.000618017, 0.000657758, 0.000699168, 0.000742285, 0.000787148, 0.000833799, 0.000882278, 0.000932629, 0.000984895, 0.00103912,
//0.00109536, 0.00115364, 0.00121404, 0.00127658, 0.00134133, 0.00140834, 0.00147765, 0.00154933, 0.00162343, 0.0017,
//0.00177911, 0.00186082, 0.00194517, 0.00203225, 0.0021221, 0.00221479, 0.00231039, 0.00240896, 0.00251057, 0.00261528,
//0.00272318, 0.00283432, 0.00294878, 0.00306663, 0.00318795, 0.00331281, 0.00344129, 0.00357346, 0.0037094, 0.0038492,
//0.00399292, 0.00414066, 0.0042925, 0.00444852, 0.0046088, 0.00477344, 0.00494251, 0.00511611, 0.00529433, 0.00547725,
//0.00566497, 0.00585758, 0.00605517, 0.00625784, 0.00646568, 0.00667879, 0.00689726, 0.0071212, 0.0073507, 0.00758587,
//0.00782681, 0.00807361, 0.00832638, 0.00858523, 0.00885026, 0.00912158, 0.00939929, 0.0096835, 0.00997433, 0.0102719,
//0.0105762, 0.0108876, 0.0112059, 0.0115315, 0.0118643, 0.0122045, 0.0125523, 0.0129076, 0.0132707, 0.0136417,
//0.0140207, 0.0144077, 0.014803, 0.0152067, 0.0156188, 0.0160394, 0.0164688, 0.0169071, 0.0173543, 0.0178105,
//0.018276, 0.0187509, 0.0192352, 0.0197291, 0.0202327, 0.0207461, 0.0212695, 0.0218031, 0.0223469, 0.022901,
//0.0234657, 0.0240409, 0.024627, 0.0252239, 0.0258318, 0.026451, 0.0270814, 0.0277232, 0.0283766, 0.0290417,
//0.0297186, 0.0304074, 0.0311084, 0.0318216, 0.0325472, 0.0332853, 0.034036, 0.0347994, 0.0355758, 0.0363652,
//0.0371678, 0.0379838, 0.0388131, 0.0396561, 0.0405127, 0.0413832, 0.0422677, 0.0431664, 0.0440792, 0.0450065,
//0.0459483, 0.0469047, 0.047876, 0.0488621, 0.0498634, 0.0508798, 0.0519115, 0.0529586, 0.0540214, 0.0550998,
//0.0561941, 0.0573044, 0.0584308, 0.0595733, 0.0607323, 0.0619077, 0.0630997, 0.0643084, 0.065534, 0.0667766,
//0.0680362, 0.0693131, 0.0706073, 0.071919, 0.0732482, 0.0745952, 0.0759599, 0.0773426, 0.0787433, 0.0801621,
//0.0815992, 0.0830547, 0.0845286, 0.0860212, 0.0875324, 0.0890625, 0.0906114, 0.0921793, 0.0937664, 0.0953726,
//0.0969982, 0.0986432, 0.100308, 0.101992, 0.103695, 0.105419, 0.107162, 0.108925, 0.110709, 0.112512,
//0.114336, 0.11618, 0.118044, 0.119928, 0.121833, 0.123759, 0.125705, 0.127672, 0.12966, 0.131669,
//0.133698, 0.135748, 0.13782, 0.139912, 0.142026, 0.14416, 0.146316, 0.148493, 0.150692, 0.152911,
//0.155152, 0.157415, 0.159699, 0.162004, 0.164331, 0.166679, 0.169048, 0.17144, 0.173853, 0.176287,
//0.178743, 0.18122, 0.183719, 0.18624, 0.188782, 0.191346, 0.193931, 0.196538, 0.199167, 0.201817,
//0.204488, 0.207181, 0.209895, 0.212631, 0.215388, 0.218167, 0.220966, 0.223787, 0.22663, 0.229493,
//0.232378, 0.235283, 0.23821, 0.241158, 0.244126, 0.247115, 0.250125, 0.253156, 0.256207, 0.259279,
//0.262371, 0.265484, 0.268616, 0.271769, 0.274942, 0.278135, 0.281347, 0.28458, 0.287832, 0.291103,
//0.294394, 0.297704, 0.301033, 0.304381, 0.307748, 0.311134, 0.314538, 0.31796, 0.321401, 0.32486,
//0.328337, 0.331832, 0.335344, 0.338874, 0.342421, 0.345985, 0.349566, 0.353164, 0.356779, 0.360409,
//0.364056, 0.36772, 0.371398, 0.375093, 0.378802, 0.382527, 0.386267, 0.390022, 0.393791, 0.397575,
//0.401372, 0.405183, 0.409008, 0.412847, 0.416698, 0.420563, 0.42444, 0.428329, 0.43223, 0.436144,
//0.440069, 0.444005, 0.447952, 0.45191, 0.455879, 0.459858, 0.463847, 0.467846, 0.471854, 0.475871,
//0.479897, 0.483931, 0.487974, 0.492025, 0.496083, 0.500148, 0.504221, 0.5083, 0.512386, 0.516478,
//0.520575, 0.524678, 0.528786, 0.532898, 0.537016, 0.541137, 0.545262, 0.54939, 0.553522, 0.557656,
//0.561793, 0.565931, 0.570072, 0.574213, 0.578356, 0.582499, 0.586643, 0.590786, 0.594929, 0.599071,
//0.603213, 0.607352, 0.61149, 0.615625, 0.619758, 0.623887, 0.628013, 0.632136, 0.636254, 0.640368,
//0.644476, 0.64858, 0.652678, 0.656769, 0.660855, 0.664933, 0.669004, 0.673068, 0.677123, 0.68117,
//0.685208, 0.689237, 0.693257, 0.697267, 0.701266, 0.705254, 0.709231, 0.713197, 0.717151, 0.721092,
//0.725021, 0.728936, 0.732838, 0.736727, 0.7406, 0.744459, 0.748303, 0.752132, 0.755944, 0.759741,
//0.76352, 0.767283, 0.771028, 0.774755, 0.778464, 0.782154, 0.785826, 0.789477, 0.79311, 0.796721,
//0.800313, 0.803883, 0.807432, 0.810959, 0.814465, 0.817947, 0.821407, 0.824844, 0.828257, 0.831646,
//0.835011, 0.838351, 0.841666, 0.844955, 0.848219, 0.851456, 0.854667, 0.857851, 0.861008, 0.864138,
//0.867239, 0.870312, 0.873357, 0.876372, 0.879358, 0.882315, 0.885242, 0.888138, 0.891004, 0.893838,
//0.896642, 0.899414, 0.902153, 0.904861, 0.907536, 0.910179, 0.912788, 0.915364, 0.917906, 0.920414,
//0.922887, 0.925326, 0.927731, 0.9301, 0.932433, 0.934731, 0.936993, 0.939219, 0.941408, 0.94356,
//0.945675, 0.947754, 0.949794, 0.951797, 0.953762, 0.955689, 0.957577, 0.959426, 0.961237, 0.963009,
//0.964741, 0.966434, 0.968087, 0.969701, 0.971274, 0.972807, 0.974299, 0.975751, 0.977162, 0.978532,
//0.979861, 0.981149, 0.982395, 0.983599, 0.984762, 0.985883, 0.986962, 0.987999, 0.988994, 0.989946,
//0.990855, 0.991722, 0.992547, 0.993328, 0.994067, 0.994763, 0.995415, 0.996025, 0.996591, 0.997114,
//0.997594, 0.99803, 0.998422, 0.998772, 0.999077, 0.999339, 0.999558, 0.999732, 0.999863, 0.999951,
//0.999995};

//const float hammingTableFloat[512] = { // nuttal
//-2.42861e-17,
//4.50915e-07, 1.80423e-06, 4.06165e-06, 7.22602e-06, 1.13013e-05, 1.62927e-05, 2.22063e-05, 2.90497e-05, 3.68313e-05, 4.55608e-05,
//5.5249e-05, 6.59079e-05, 7.75505e-05, 9.01912e-05, 0.000103845, 0.000118529, 0.00013426, 0.000151058, 0.000168942, 0.000187933,
//0.000208054, 0.000229327, 0.000251778, 0.000275432, 0.000300315, 0.000326456, 0.000353884, 0.000382629, 0.000412722, 0.000444196,
//0.000477085, 0.000511423, 0.000547245, 0.000584591, 0.000623496, 0.000664002, 0.000706147, 0.000749975, 0.000795528, 0.000842849,
//0.000891985, 0.00094298, 0.000995883, 0.00105074, 0.00110761, 0.00116653, 0.00122756, 0.00129075, 0.00135616, 0.00142384,
//0.00149385, 0.00156624, 0.00164108, 0.00171842, 0.00179833, 0.00188086, 0.00196609, 0.00205408, 0.00214488, 0.00223857,
//0.00233522, 0.0024349, 0.00253767, 0.00264361, 0.00275279, 0.00286528, 0.00298116, 0.0031005, 0.00322339, 0.00334989,
//0.00348009, 0.00361407, 0.00375191, 0.00389369, 0.0040395, 0.00418942, 0.00434353, 0.00450193, 0.0046647, 0.00483193,
//0.00500371, 0.00518013, 0.00536129, 0.00554727, 0.00573818, 0.0059341, 0.00613513, 0.00634138, 0.00655294, 0.0067699,
//0.00699238, 0.00722047, 0.00745427, 0.00769389, 0.00793943, 0.008191, 0.0084487, 0.00871264, 0.00898294, 0.00925969,
//0.00954302, 0.00983302, 0.0101298, 0.0104335, 0.0107442, 0.0110621, 0.0113872, 0.0117197, 0.0120597, 0.0124072,
//0.0127625, 0.0131257, 0.0134967, 0.0138759, 0.0142633, 0.014659, 0.0150632, 0.0154759, 0.0158974, 0.0163277,
//0.0167669, 0.0172152, 0.0176728, 0.0181396, 0.018616, 0.0191019, 0.0195976, 0.0201031, 0.0206186, 0.0211443,
//0.0216801, 0.0222264, 0.0227832, 0.0233507, 0.023929, 0.0245182, 0.0251184, 0.0257298, 0.0263526, 0.0269869,
//0.0276327, 0.0282903, 0.0289598, 0.0296413, 0.0303349, 0.0310409, 0.0317592, 0.0324902, 0.0332338, 0.0339903,
//0.0347598, 0.0355424, 0.0363383, 0.0371476, 0.0379704, 0.0388068, 0.0396571, 0.0405214, 0.0413997, 0.0422923,
//0.0431993, 0.0441207, 0.0450568, 0.0460077, 0.0469735, 0.0479543, 0.0489503, 0.0499617, 0.0509885, 0.0520309,
//0.053089, 0.054163, 0.055253, 0.0563592, 0.0574815, 0.0586203, 0.0597756, 0.0609476, 0.0621363, 0.0633419,
//0.0645646, 0.0658045, 0.0670616, 0.0683361, 0.0696282, 0.070938, 0.0722655, 0.0736109, 0.0749744, 0.076356,
//0.0777559, 0.0791741, 0.0806108, 0.0820662, 0.0835403, 0.0850331, 0.086545, 0.0880759, 0.0896259, 0.0911952,
//0.0927839, 0.094392, 0.0960198, 0.0976671, 0.0993343, 0.101021, 0.102728, 0.104455, 0.106203, 0.10797,
//0.109758, 0.111566, 0.113394, 0.115244, 0.117113, 0.119004, 0.120915, 0.122848, 0.124801, 0.126775,
//0.12877, 0.130787, 0.132824, 0.134883, 0.136964, 0.139065, 0.141188, 0.143333, 0.145499, 0.147687,
//0.149896, 0.152127, 0.15438, 0.156654, 0.15895, 0.161268, 0.163608, 0.16597, 0.168353, 0.170759,
//0.173186, 0.175635, 0.178106, 0.1806, 0.183115, 0.185652, 0.188211, 0.190792, 0.193394, 0.196019,
//0.198666, 0.201334, 0.204025, 0.206737, 0.209471, 0.212227, 0.215005, 0.217804, 0.220625, 0.223468,
//0.226332, 0.229218, 0.232125, 0.235054, 0.238004, 0.240975, 0.243968, 0.246981, 0.250016, 0.253072,
//0.256149, 0.259247, 0.262365, 0.265504, 0.268664, 0.271844, 0.275044, 0.278265, 0.281506, 0.284767,
//0.288048, 0.291349, 0.294669, 0.298009, 0.301368, 0.304747, 0.308145, 0.311562, 0.314997, 0.318452,
//0.321925, 0.325416, 0.328925, 0.332453, 0.335999, 0.339562, 0.343143, 0.346741, 0.350356, 0.353988,
//0.357637, 0.361303, 0.364985, 0.368684, 0.372398, 0.376128, 0.379874, 0.383635, 0.387411, 0.391202,
//0.395008, 0.398829, 0.402663, 0.406512, 0.410374, 0.41425, 0.418139, 0.422041, 0.425956, 0.429883,
//0.433822, 0.437774, 0.441737, 0.445712, 0.449697, 0.453694, 0.457701, 0.461718, 0.465746, 0.469783,
//0.47383, 0.477885, 0.48195, 0.486023, 0.490104, 0.494193, 0.49829, 0.502394, 0.506505, 0.510622,
//0.514746, 0.518876, 0.523011, 0.527152, 0.531298, 0.535448, 0.539602, 0.543761, 0.547923, 0.552089,
//0.556257, 0.560428, 0.564601, 0.568776, 0.572952, 0.577129, 0.581307, 0.585486, 0.589665, 0.593843,
//0.59802, 0.602197, 0.606371, 0.610544, 0.614715, 0.618883, 0.623048, 0.62721, 0.631368, 0.635522,
//0.639671, 0.643815, 0.647954, 0.652087, 0.656214, 0.660335, 0.664448, 0.668554, 0.672653, 0.676743,
//0.680825, 0.684898, 0.688962, 0.693016, 0.697059, 0.701093, 0.705115, 0.709126, 0.713125, 0.717112,
//0.721087, 0.725049, 0.728997, 0.732931, 0.736852, 0.740757, 0.744648, 0.748523, 0.752383, 0.756226,
//0.760053, 0.763863, 0.767655, 0.77143, 0.775186, 0.778924, 0.782643, 0.786342, 0.790021, 0.793681,
//0.79732, 0.800937, 0.804534, 0.808108, 0.811661, 0.815191, 0.818697, 0.822181, 0.825641, 0.829077,
//0.832488, 0.835875, 0.839236, 0.842572, 0.845881, 0.849165, 0.852421, 0.855651, 0.858853, 0.862027,
//0.865173, 0.868291, 0.87138, 0.874439, 0.877469, 0.880469, 0.883439, 0.886378, 0.889286, 0.892163,
//0.895009, 0.897822, 0.900603, 0.903352, 0.906067, 0.90875, 0.911399, 0.914014, 0.916595, 0.919141,
//0.921653, 0.92413, 0.926571, 0.928977, 0.931346, 0.93368, 0.935977, 0.938238, 0.940461, 0.942647,
//0.944796, 0.946907, 0.94898, 0.951014, 0.953011, 0.954968, 0.956886, 0.958765, 0.960605, 0.962405,
//0.964165, 0.965885, 0.967565, 0.969205, 0.970803, 0.972361, 0.973877, 0.975353, 0.976787, 0.978179,
//0.97953, 0.980838, 0.982105, 0.983329, 0.984511, 0.98565, 0.986747, 0.987801, 0.988812, 0.98978,
//0.990704, 0.991586, 0.992424, 0.993218, 0.993969, 0.994676, 0.995339, 0.995959, 0.996535, 0.997066,
//0.997554, 0.997997, 0.998396, 0.998751, 0.999062, 0.999328, 0.99955, 0.999728, 0.999861, 0.99995,
//0.999994};

const float hammingTableFloat[512] = //HAMMING
		{ 0.08, 0.0800087, 0.0800347, 0.0800781, 0.0801388, 0.0802169, 0.0803123, 0.0804251, 0.0805552, 0.0807026,
				0.0808674, 0.0810494, 0.0812488, 0.0814655, 0.0816995, 0.0819508, 0.0822194, 0.0825052, 0.0828083,
				0.0831286, 0.0834662, 0.083821, 0.084193, 0.0845822, 0.0849885, 0.0854121, 0.0858527, 0.0863106,
				0.0867855, 0.0872775, 0.0877866, 0.0883128, 0.088856, 0.0894162, 0.0899934, 0.0905876, 0.0911988,
				0.0918269, 0.0924719, 0.0931337, 0.0938125, 0.094508, 0.0952204, 0.0959495, 0.0966954, 0.097458,
				0.0982373, 0.0990333, 0.0998459, 0.100675, 0.101521, 0.102383, 0.103262, 0.104157, 0.105069, 0.105997,
				0.106942, 0.107903, 0.10888, 0.109873, 0.110883, 0.111909, 0.112951, 0.114009, 0.115083, 0.116173,
				0.117279, 0.118402, 0.11954, 0.120693, 0.121863, 0.123049, 0.12425, 0.125467, 0.126699, 0.127947,
				0.129211, 0.13049, 0.131785, 0.133095, 0.13442, 0.135761, 0.137117, 0.138488, 0.139874, 0.141276,
				0.142692, 0.144123, 0.14557, 0.147031, 0.148507, 0.149998, 0.151503, 0.153023, 0.154558, 0.156107,
				0.157671, 0.159249, 0.160842, 0.162449, 0.16407, 0.165705, 0.167355, 0.169018, 0.170696, 0.172387,
				0.174092, 0.175811, 0.177544, 0.179291, 0.181051, 0.182824, 0.184611, 0.186412, 0.188226, 0.190053,
				0.191893, 0.193747, 0.195613, 0.197493, 0.199385, 0.20129, 0.203208, 0.205139, 0.207082, 0.209038,
				0.211007, 0.212988, 0.214981, 0.216986, 0.219004, 0.221033, 0.223075, 0.225129, 0.227195, 0.229272,
				0.231361, 0.233462, 0.235574, 0.237698, 0.239833, 0.24198, 0.244137, 0.246306, 0.248486, 0.250677,
				0.252879, 0.255092, 0.257315, 0.25955, 0.261794, 0.26405, 0.266315, 0.268591, 0.270877, 0.273174,
				0.27548, 0.277797, 0.280123, 0.282459, 0.284805, 0.28716, 0.289525, 0.2919, 0.294283, 0.296677,
				0.299079, 0.30149, 0.30391, 0.30634, 0.308778, 0.311224, 0.31368, 0.316144, 0.318616, 0.321097,
				0.323586, 0.326083, 0.328588, 0.331101, 0.333623, 0.336151, 0.338688, 0.341232, 0.343784, 0.346343,
				0.348909, 0.351483, 0.354063, 0.356651, 0.359246, 0.361847, 0.364455, 0.36707, 0.369691, 0.372319,
				0.374953, 0.377593, 0.38024, 0.382892, 0.38555, 0.388215, 0.390884, 0.39356, 0.396241, 0.398927,
				0.401619, 0.404316, 0.407018, 0.409725, 0.412438, 0.415154, 0.417876, 0.420602, 0.423333, 0.426068,
				0.428807, 0.431551, 0.434299, 0.43705, 0.439806, 0.442565, 0.445328, 0.448095, 0.450865, 0.453638,
				0.456415, 0.459195, 0.461977, 0.464763, 0.467552, 0.470343, 0.473137, 0.475934, 0.478733, 0.481534,
				0.484337, 0.487143, 0.48995, 0.49276, 0.495571, 0.498384, 0.501199, 0.504014, 0.506832, 0.50965,
				0.51247, 0.515291, 0.518112, 0.520935, 0.523758, 0.526582, 0.529406, 0.532231, 0.535056, 0.537881,
				0.540706, 0.543532, 0.546357, 0.549182, 0.552006, 0.55483, 0.557654, 0.560477, 0.563299, 0.56612,
				0.56894, 0.571759, 0.574577, 0.577394, 0.580209, 0.583023, 0.585835, 0.588645, 0.591454, 0.59426,
				0.597065, 0.599867, 0.602667, 0.605465, 0.60826, 0.611053, 0.613843, 0.61663, 0.619414, 0.622196,
				0.624974, 0.627749, 0.630521, 0.633289, 0.636054, 0.638815, 0.641572, 0.644326, 0.647076, 0.649821,
				0.652563, 0.6553, 0.658033, 0.660762, 0.663485, 0.666205, 0.668919, 0.671629, 0.674333, 0.677033,
				0.679727, 0.682416, 0.6851, 0.687779, 0.690451, 0.693118, 0.69578, 0.698435, 0.701084, 0.703728,
				0.706365, 0.708996, 0.71162, 0.714238, 0.71685, 0.719455, 0.722053, 0.724644, 0.727228, 0.729805,
				0.732375, 0.734938, 0.737493, 0.740041, 0.742581, 0.745114, 0.747639, 0.750156, 0.752665, 0.755167,
				0.75766, 0.760145, 0.762621, 0.765089, 0.767549, 0.77, 0.772442, 0.774876, 0.777301, 0.779717, 0.782123,
				0.784521, 0.78691, 0.789289, 0.791658, 0.794019, 0.796369, 0.79871, 0.801041, 0.803363, 0.805674,
				0.807976, 0.810267, 0.812548, 0.814819, 0.817079, 0.819329, 0.821569, 0.823798, 0.826016, 0.828223,
				0.83042, 0.832605, 0.83478, 0.836943, 0.839095, 0.841236, 0.843365, 0.845484, 0.84759, 0.849685,
				0.851768, 0.85384, 0.855899, 0.857947, 0.859983, 0.862007, 0.864018, 0.866017, 0.868004, 0.869979,
				0.871941, 0.873891, 0.875828, 0.877752, 0.879664, 0.881563, 0.883449, 0.885322, 0.887182, 0.889029,
				0.890862, 0.892683, 0.89449, 0.896284, 0.898064, 0.899831, 0.901584, 0.903324, 0.90505, 0.906762,
				0.908461, 0.910145, 0.911815, 0.913472, 0.915114, 0.916742, 0.918356, 0.919956, 0.921542, 0.923112,
				0.924669, 0.926211, 0.927739, 0.929251, 0.93075, 0.932233, 0.933702, 0.935155, 0.936594, 0.938018,
				0.939427, 0.940821, 0.942199, 0.943563, 0.944911, 0.946244, 0.947562, 0.948864, 0.950151, 0.951423,
				0.952679, 0.953919, 0.955144, 0.956353, 0.957546, 0.958724, 0.959885, 0.961031, 0.962162, 0.963276,
				0.964374, 0.965456, 0.966522, 0.967572, 0.968606, 0.969624, 0.970625, 0.971611, 0.97258, 0.973533,
				0.974469, 0.975389, 0.976292, 0.977179, 0.97805, 0.978904, 0.979742, 0.980562, 0.981367, 0.982154,
				0.982925, 0.98368, 0.984417, 0.985138, 0.985842, 0.986529, 0.987199, 0.987853, 0.988489, 0.989109,
				0.989712, 0.990297, 0.990866, 0.991418, 0.991952, 0.99247, 0.992971, 0.993454, 0.99392, 0.99437,
				0.994802, 0.995217, 0.995615, 0.995995, 0.996359, 0.996705, 0.997034, 0.997345, 0.99764, 0.997917,
				0.998177, 0.99842, 0.998645, 0.998853, 0.999044, 0.999217, 0.999373, 0.999512, 0.999633, 0.999738,
				0.999824, 0.999894, 0.999946, 0.99998, 0.999998 };

// Apply a Hamming window to the input sample data
void applyHammingWindowFloat(float *fx)
{
	uint16_t i = 0;

	// First half of the window
	for (int16_t k = 0; k < FHT_LEN / 2; ++k)
	{
		fx[i] *= hammingTableFloat[k];
		i++;
	}

	// Second half of the window
	for (int16_t k = ((FHT_LEN / 2) - 1); k >= 0; --k)
	{
		fx[i] *= hammingTableFloat[k];
		i++;
	}

}
