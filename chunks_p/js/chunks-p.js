/**
 * @file Provide a client-side theme implementation for text chunks.
 */

(function($) {
  Drupal.theme.prototype.chunk__p = function(config) {
    var output = '<div class="chunk p-chunk">';
    output += Drupal.checkPlain(config.p);
    output += '</div>';
    return output;
  };
  Drupal.theme.prototype.chunk_callback__p = function(config, classPrepend) {
    $(classPrepend + 'preview .p-chunk').each(function() {
      // Initialize editor for this Paragraph chunk
      var editor,
          editorConfig;

      editorConfig = {
        class: 'pen-editor',
        debug: true,
        editor: this,
        textarea: '<textarea name="content"></textarea>', // fallback for old browsers
        list: ['bold', 'italic', 'underline', 'createlink']
      };
      editor = new Pen(editorConfig);
    });
  };
})(jQuery);