Providers 0.1
----------------------

Summary
-------
Providers query webservices like Facebook, Google+, Flickr, YouTube, Twitter, etc,
and import the items returned from API calls as nodes in your Drupal site. There 
are many advantages to this:

1) You have complete control over the design of these items. For example, if you
have a Twitter widget, Twitter's branding will appear around the widget. If
instead you use a Twitter provider then you can create your own templates in Drupal
to display tweets however you want.

2) If a webservice becomes unavailable you will still be able to display content
imported from that service.

3) You won't need to include external Javascripts or make API calls with each page
load. This will speed up the page load time on your site.

4) You can order, filter, and display content in ways that an API may not allow.

This module only acts as an API for other modules to implement. The next section
lists providers that have been created.


Providers
---------
YouTube
 Drupal: https://drupal.org/sandbox/jtrost/2049661
 Github: https://github.com/jtrost/provider_youtube


Creating a new Provider
-----------------------
Creating a new provider has 3 parts: Defining the provider, querying the API, and
saving nodes for each item returned by the API. This module creates two hooks: 
hook_fetch_provider() and hook_set_provider_types(). The easiest way to create a provider
is to use the YouTube Provider module as a boilerplate[5].

1) Define the provider[6].
2) Create a curl function[7].
3) Create a function that assigns the data from each item to fields in the content type[8].
4) Implement hook_fetch_provider(). This function is responsible for running the provider.
 * Load the provider[9].
 * Build an URL to curl[10].
 * Run the curl and create new nodes for each item[11].
 * Create reporting / user feedback to let the user know the import was successful.


Authors
-------
jtrost


Footnotes
---------
[1] https://developers.google.com/youtube/2.0/developers_guide_protocol_video_feeds#User_Uploaded_Videos
[2] https://drupal.org/sandbox/jtrost/2049659
[3] https://developers.google.com/youtube/2.0/reference?csw=1#max-resultssp
[4] https://developers.google.com/youtube/2.0/reference?csw=1#start-indexsp
[5] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module
[6] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L100-L117
[7] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L122-L129
[8] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L69-L95
[9] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L7
[10] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L10
[11] https://github.com/jtrost/provider_youtube/blob/master/provider_youtube.module#L17-L42