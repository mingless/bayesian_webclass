
var apk = angular.module('apk',	[]);
apk.
controller('controller1', function($scope) {

	$scope.link='';
	$scope.response = 'placeholder1';
	$scope.dataFromServer='no data!';
	$scope.sendWebAddress = function(){

		$scope.response = 'placeholder2';			
	};

})
.controller('cppController',
		['$scope', 'srvCommands',
			function($scope, srvCommands) {
				$scope.dataFromServer = 'no data!';

				$scope.getData = function() {
				
					srvCommands.communicateWithServer(
						function(data){
							$scope.dataFromServer=data.toString();
						}
					);
				};
			
			}
		]		

);






