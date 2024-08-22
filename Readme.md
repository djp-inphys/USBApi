  <div class="headertitle">
<div class="title">VtAPI Documentation</div>  </div>
</div><div class="contents">
<div class="textblock"><p>Copyright (c) 2013 by Innovative Physics All Rights Reserved</p>
<div class="image">
<img src="ipl_logo.png" alt=""/>
</div>
<dl class="section author"><dt>Author</dt><dd>David Prendergast</dd></dl>
<h1><a class="anchor" id="sec1"></a>
Innovative Physics Sensor API</h1>
<dl class="section user"><dt>REQUIREMENTS: </dt><dd>The Innovative Physics API is implemented as an abstract class factory. The class factory generates the concrete classes corresponding to the abstract API classes. The abstract API classes Vt::CVtAPI, <a class="el" href="class_vt_1_1_c_vtpc_a_p_i.html">Vt::CVtpcAPI</a> and <a class="el" href="class_vt_1_1_c_vthds_a_p_i.html" title="Main hds API class.">Vt::CVthdsAPI</a>. are defined in three header files, <a class="el" href="_vt_a_p_i_8h.html">VtAPI.h</a>, <a class="el" href="_vtpc_a_p_i_8h.html">VtpcAPI.h</a> and <a class="el" href="_vthds_a_p_i_8h.html">VthdsAPI.h</a>.</dd></dl>
<dl class="section user"><dt></dt><dd>To use the system all that is required are three steps, <pre class="fragment">          -# include the VtAPI.h header file.
          -# make a call to Vt::GetAPI() somewhere in your code e.g.
                CVtAPI &amp;API = GetAPI( CVtAPI::HDS20_API );
          -# use the API reference to do something e.g.
                  API.capture();
</pre></dd></dl>
<dl class="section user"><dt>SPECIFICATIONS: </dt><dd>Create the appropriate concrete objects for the implementation of the API. </dd></dl>
<dl class="section user"><dt></dt><dd>Currently the construction of the concrete object uses the following structure </dd></dl>
<dl class="section user"><dt></dt><dd>1) Create the pipe_data object, this provides an abstraction for the acquired data The pipe data object is passed to the parser. The pipe object is generic. It is used for all types of raw data ie. pano ceph and hds. The parser is specfic to the API type, there are currently two types of parser, one which understands data coming from the pano and ceph sensors and one that understands hds sensor data. </dd></dl>
<dl class="section user"><dt></dt><dd>2) Once the parser is created this is passed to the driver object. Like the pipe data object the driver object is generic and used to access the hardware for all types of interface. The driver object encapsulates everything to do with the hardware. Effectively, the driver forms the basis of a high level hardware abstraction layer. The driver is responsible for acquiring the data, by filling in the information in pipedata. Once pipe data has been filled with data the parser is responsible for interpriting this data and turning it into individual lines of the acquired image. Clearly. the parser must know the structure of the data coming from the various sources. Consequently, the parser type is specific to the particular API type instantiated. Once the data has been parsed it is placed in a dataset by the USB driver.</dd></dl>
<dl class="section note"><dt>Note</dt><dd>Error handling is managed via the Vt_fail exception generation. Hence, any object calling GetAPI, should catch any potential exceptions.</dd></dl>
<dl class="section user"><dt></dt><dd>The Sys object in addition to being an abstract class factory is implemented around a singleton design pattern. That is the sys object is a singleton. However, rather than returning an instance of itself, as might be usual, it returns a copy of the generate API classes contained within itself. Consequently, any subsequent call to GetAPI after the initial call will return a reference to the class created on the first call.</dd></dl>
<dl class="section user"><dt>Initialisation. </dt><dd>The objects created within the system object are not automatically initialised. It is the responsibility of the class user to initialise the API i.e. any attempt to use the class should first be proceed by a call to the generic API.init() function.</dd></dl>