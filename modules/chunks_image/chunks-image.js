/**
 * @file improve user interaction with the image chunk widgets.
 */

(function($) {
  Drupal.behaviors.chunksImageCallbacks = {
    attach: function(context, settings) {

      if (typeof Drupal.settings.chunks.callbacks.initialize.image === 'undefined') {
        // Implements the initialize callback to perform actions on a image
        // Chunk object upon initialization.
        Drupal.settings.chunks.callbacks.initialize.image = function(chunk) {

          // Only perform certain actions if the current chunk is not assigned
          // to a different type.
          if (chunk.chunkType === '' || chunk.chunkType === 'image') {

            // Hide the cancel button if you remove an image. If the user were to
            // press cancel it would seem as if their previously uploaded image would
            // remain in the chunk untouched. This is not the case, however, since
            // that file was actually removed from the files directory.
            $('.image-widget .form-submit[value="' + Drupal.t('Remove') + '"]', chunk.element).bind('mousedown.removeChunkImage', function(e) {
              if (e.which === 1) {
                chunk.element.find('.chunk-cancel-button').hide();
              }
            });

            // Set focus to image style selection after pressing "Upload" button for
            // better usability.
            $('.image-widget .form-submit[value="' + Drupal.t('Upload') + '"]', context).bind('mousedown.removeChunkImage', function(e) {
              chunk.element.find('[name="' + chunk.namePrepend + '[configuration][image][image_style]"]').focus();
            });
          }
        };
      }

    }
  };
})(jQuery);
