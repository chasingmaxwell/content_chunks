/**
 * @file
 * JavaScript hooks provided by the chunks module.
 */

/**
 * Theme a chunk entirely on the client through JavaScript.
 *
 * @param {dictionary} configuration
 *   A dictionary containing the same configuration data which is passed to the
 *   regular theme hook in Drupal.
 *
 * @return {string}
 *   A string of html markup to be inserted into the DOM as the chunk's preview.
 */
Drupal.theme.prototype.chunk__CHUNK_TYPE = function(configuration) {
  // In this case we return a single configuration property as plain text passed
  // through Drupal.checkPlain() for security.
  var output = '<div class="chunk text-chunk">';
  output += Drupal.checkPlain(configuration.text);
  output += '</div>';
  return output;
};

