<div align="center">
<img src="doc/picture/cc.png" width = "50%" height = "50%" alt="logo" align=center/>
</div>

[Chinese README](./README_ZH_CN.md)

## What is MegCC

MegCC is a deep-learning model compiler with the following features:
* **Extremely Lightweight Runtime**: Only keep the required computation kernel in your binary. e.g., **81KB** runtime for MobileNet v1
* **High Performance**: Every operation is carefully optimized by experts
* **Portable**: generate nothing but computation code, easy to compile and use on Linux, Android, TEE, BareMetal
* **Low Memory Usage while Boot Instantly**: Model optimization and memory planning are generated at compile time. Get State-of-the-art level memory usage and spend no extra CPU during inference

MegCC compiler is developed based on MLIR infrastructure. Most of the code generated by the compiler is optimized by hand. MegCC supports neural networks that contain tensors in static shape or dynamic shape.
To help achieve the minimum binary size, it also supports generating the necessary CV operators so that you don't need to link another giant CV lib.

When compiling a model:
* MegCC generates both the kernels used by the model and user-required CV kernels
* MegCC does several optimizations, such as static memory planning and model optimization
* MegCC dumps the data above into the final model

MegCC runtime loads the model and uses the generated kernels to finish the model inference. Only 81KB binary size is required to inference MobileNetV1 (in fp32).

MegCC supports Arm64/ArmV7/X86/BareMatal backend. You may want to check [supported operator lists](doc/opr.md).

### MegCC Structure
![megcc_struct](doc/picture/megcc.png)

## Documentation

#### Get MegCC
* Download release compiler suit from [release page](https://github.com/MegEngine/MegCC/releases)
* Compiler from source, please fellow the [compiler doc](compiler/README.md)
* Build the release tar, please fellow the [release doc](doc/how-to-release.md)

#### How to use MegCC

* Read [how-to-use](doc/how-to-use.md) to see how to compile your models and deploy them，also there is a Engilish doc [how to use](doc/how-to-use.md).
* MegCC runtime is easy to run in standard OS, even no OS([example](runtime/example/README.md)).  
 
**Thanks a lot, please enjoy it**
